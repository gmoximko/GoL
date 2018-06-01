#include <sstream>

#include <QPoint>
#include <QDebug>
#include <QTimerEvent>

#include "steamnetwork.h"

namespace Network {

namespace {

constexpr auto const* c_lobby_name = "name";
constexpr auto const* c_field_size = "field_size";
constexpr auto const* c_game_speed = "game_speed";
constexpr auto const* c_rich_status = "status";
constexpr auto const c_update_time = 30;

template<typename T>
std::string toString(T value)
{
  std::ostringstream stream;
  stream << value;
  return stream.str();
}

std::string toString(QPoint const& point)
{
  std::ostringstream stream;
  stream << point.x() << ' ' << point.y();
  return stream.str();
}

template<typename T>
T toValue(std::string const& str);

template<>
QPoint toValue(std::string const& str)
{
  std::istringstream stream(str);
  int x = 0;
  int y = 0;
  stream >> x >> y;
  return QPoint(x, y);
}

class Lobby
{
public:
  static void join(CSteamID lobby_id, Logic::GameParamsPtr game_params)
  {}
  static void create(CSteamID lobby_id, Logic::GameParamsPtr game_params)
  {
    SteamMatchmaking()->SetLobbyData(lobby_id, c_lobby_name, SteamFriends()->GetPersonaName());
    SteamMatchmaking()->SetLobbyData(lobby_id, c_field_size, toString(game_params->fieldSize()).c_str());
    SteamMatchmaking()->SetLobbyData(lobby_id, c_game_speed, toString(game_params->gameSpeed()).c_str());
  }

  Lobby(Lobby const& lobby) = delete;
  void operator=(Lobby const& lobby) = delete;

private:
  explicit Lobby(CSteamID lobby_id, Logic::GameParamsPtr game_params)
    : lobby_id_(lobby_id)
    , game_params_(std::move(game_params))
    , on_persona_state_changed_(this, &Lobby::onPersonaStateChanged)
    , on_lobby_data_updated_(this, &Lobby::onLobbyDataUpdated)
    , on_chat_data_updated_(this, &Lobby::onLobbyChatUpdated)
    , on_p2p_session_requested_(this, &Lobby::onP2PSessionRequested)
  {}

  CSteamID const lobby_id_;
  Logic::GameParamsPtr game_params_;

  STEAM_CALLBACK(Lobby, onPersonaStateChanged, PersonaStateChange_t, on_persona_state_changed_);
  STEAM_CALLBACK(Lobby, onLobbyDataUpdated, LobbyDataUpdate_t, on_lobby_data_updated_);
  STEAM_CALLBACK(Lobby, onLobbyChatUpdated, LobbyChatUpdate_t, on_chat_data_updated_);
  STEAM_CALLBACK(Lobby, onP2PSessionRequested, P2PSessionRequest_t, on_p2p_session_requested_);
};

void Lobby::onPersonaStateChanged(PersonaStateChange_t* callback)
{}

void Lobby::onLobbyDataUpdated(LobbyDataUpdate_t* callback)
{}

void Lobby::onLobbyChatUpdated(LobbyChatUpdate_t* callback)
{
  if (lobby_id_ != callback->m_ulSteamIDLobby)
  {
    return;
  }
  auto const id = CSteamID(callback->m_ulSteamIDUserChanged);
  switch (callback->m_rgfChatMemberStateChange)
  {
  case k_EChatMemberStateChangeEntered:
    qDebug() << SteamFriends()->GetFriendPersonaName(id) << "joined lobby!";
    break;
  case k_EChatMemberStateChangeLeft:
  default:
    qDebug() << SteamFriends()->GetFriendPersonaName(id) << "left lobby!";
    auto const result = SteamNetworking()->CloseP2PSessionWithUser(id);
    Q_ASSERT(result);
    break;
  }
}

void Lobby::onP2PSessionRequested(P2PSessionRequest_t* callback)
{
  auto const lobby_member_count = SteamMatchmaking()->GetNumLobbyMembers(lobby_id_);
  for (int idx = 0; idx < lobby_member_count; ++idx)
  {
    auto const lobby_member = SteamMatchmaking()->GetLobbyMemberByIndex(lobby_id_, idx);
    if (callback->m_steamIDRemote == lobby_member)
    {
      auto const result = SteamNetworking()->AcceptP2PSessionWithUser(lobby_member);
      Q_ASSERT(result);
      break;
    }
  }
}

} // namespace

SteamNetwork::SteamNetwork(QObject* parent, Logic::GameParamsPtr game_params)
  : GameNetwork(parent)
  , callback_timer_id_(startTimer(c_update_time, Qt::TimerType::PreciseTimer))
  , game_params_(std::move(game_params))
{
  Q_ASSERT(callback_timer_id_ != 0);
  if (!SteamAPI_Init())
  {
    throw std::runtime_error("Steam initialization failed!");
  }
}

SteamNetwork::~SteamNetwork()
{
  SteamAPI_Shutdown();
}

void SteamNetwork::createLobby()
{
  auto const result =
      SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, game_params_->maxPlayerCount());
  on_lobby_created_.Set(result, this, &SteamNetwork::onLobbyCreated);
  SteamFriends()->SetRichPresence(c_rich_status, "Creating a lobby");
}

void SteamNetwork::joinLobby(Logic::LobbyId lobby_id)
{
  auto const result = SteamMatchmaking()->JoinLobby(CSteamID(lobby_id));
  on_lobby_entered_.Set(result, this, &SteamNetwork::onLobbyEntered);
}

void SteamNetwork::timerEvent(QTimerEvent* event)
{
  if (event->timerId() != callback_timer_id_)
  {
    return;
  }
  SteamAPI_RunCallbacks();

  if (requesting_lobbies_)
  {
    return;
  }
  requesting_lobbies_ = true;
  auto const result = SteamMatchmaking()->RequestLobbyList();
  on_lobby_list_matched_.Set(result, this, &SteamNetwork::onLobbyListMatched);
  SteamFriends()->SetRichPresence(c_rich_status, "Main menu: finding lobbies");
}

void SteamNetwork::onLobbyListMatched(LobbyMatchList_t* callback, bool failure)
{
  requesting_lobbies_ = false;
  Lobbies lobbies;

  for (size_t idx = 0; idx < callback->m_nLobbiesMatching; ++idx)
  {
    auto const steam_id = SteamMatchmaking()->GetLobbyByIndex(static_cast<int>(idx));
    auto const* lobby_name = SteamMatchmaking()->GetLobbyData(steam_id, c_lobby_name);
    if (lobby_name != nullptr)
    { 
      lobbies.push_back(QVariant::fromValue(Logic::GameParams
      {
        static_cast<Logic::LobbyId>(steam_id.ConvertToUint64()),
        QString(lobby_name)
      }));
    }
  }
  emit lobbiesUpdated(std::move(lobbies));
}

void SteamNetwork::onLobbyCreated(LobbyCreated_t* callback, bool failure)
{
  if (callback->m_eResult == k_EResultOK)
  {
    Lobby::create(callback->m_ulSteamIDLobby, game_params_);
  }
  qDebug() << "onLobbyCreated " << failure;
}

void SteamNetwork::onLobbyEntered(LobbyEnter_t* callback, bool failure)
{
  if (callback->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
  {
    Lobby::join(callback->m_ulSteamIDLobby, game_params_);
  }
  qDebug() << "onLobbyEntered " << failure;
}

GameNetworkPtr createGameNetwork(QObject* parent, Logic::GameParamsPtr game_params)
{
  return SteamAPI_RestartAppIfNecessary(480)
      ? nullptr
      : new SteamNetwork(parent, game_params);
}

} // Network
