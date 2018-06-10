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
constexpr auto const* c_player_count = "player_count";

constexpr auto const c_update_time = 1000 / 30;

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

std::string toString(QString const& str)
{
  return str.toUtf8().data();
}

template<typename T>
T toValue(std::string const& str)
{
  std::istringstream stream(str);
  auto result = T();
  stream >> result;
  return result;
}

template<>
QPoint toValue(std::string const& str)
{
  std::istringstream stream(str);
  int x = 0;
  int y = 0;
  stream >> x >> y;
  return QPoint(x, y);
}

template<>
QString toValue(std::string const& str)
{
  return str.c_str();
}

template<class Owner, typename T, T Owner::*member>
struct Member
{
  static bool read(CSteamID lobby_id, Owner& owner, char const* key)
  {
    auto const* value = SteamMatchmaking()->GetLobbyData(lobby_id, key);
    auto const result = value != nullptr && std::strcmp(value, "") != 0;
    if (result)
    {
      owner.*member = toValue<T>(value);
    }
    return result;
  }
  static bool write(CSteamID lobby_id, Owner const& owner, char const* key)
  {
    auto const str = toString(owner.*member);
    auto const result = SteamMatchmaking()->SetLobbyData(lobby_id, key, str.c_str());
    return result;
  }
};

bool retrieveLobby(CSteamID lobby_id, LobbyParams& lobby_params)
{
  using FillFunc = bool(*)(CSteamID, LobbyParams&, char const*);
  static std::map<std::string, FillFunc> const fillers
  {
    { c_lobby_name, Member<LobbyParams, QString, &LobbyParams::name_>::read },
    { c_field_size, Member<LobbyParams, QPoint, &LobbyParams::field_size_>::read },
    { c_game_speed, Member<LobbyParams, int, &LobbyParams::game_speed_>::read },
    { c_player_count, Member<LobbyParams, int, &LobbyParams::player_count_>::read },
  };

  lobby_params.lobby_id_ = static_cast<LobbyId>(lobby_id.ConvertToUint64());
  return std::all_of(fillers.begin(), fillers.end(), [lobby_id, &lobby_params](auto iter)
  {
    return iter.second(lobby_id, lobby_params, iter.first.c_str());
  });
}

bool writeLobbyParams(CSteamID lobby_id, LobbyParams const& lobby_params)
{
  using WriteFunc = bool(*)(CSteamID, LobbyParams const&, char const*);
  static std::map<std::string, WriteFunc> const writers
  {
    { c_lobby_name, Member<LobbyParams, QString, &LobbyParams::name_>::write },
    { c_field_size, Member<LobbyParams, QPoint, &LobbyParams::field_size_>::write },
    { c_game_speed, Member<LobbyParams, int, &LobbyParams::game_speed_>::write },
    { c_player_count, Member<LobbyParams, int, &LobbyParams::player_count_>::write },
  };
  return std::all_of(writers.begin(), writers.end(), [lobby_id, &lobby_params](auto iter)
  {
    return iter.second(lobby_id, lobby_params, iter.first.c_str());
  });
}

void sendP2PMsg(CSteamID id, std::string const& msg)
{
  SteamNetworking()->SendP2PPacket(id, msg.data(), static_cast<uint32_t>(msg.size()), k_EP2PSendReliable);
}

template<typename Func>
void foreachLobbyMember(CSteamID lobby_id, Func&& func)
{
  auto const lobby_member_count = SteamMatchmaking()->GetNumLobbyMembers(lobby_id);
  for (int idx = 0; idx < lobby_member_count; ++idx)
  {
    auto const lobby_member = SteamMatchmaking()->GetLobbyMemberByIndex(lobby_id, idx);
    func(lobby_member);
  }
}

} // namespace

class SteamNetwork::Lobby
{
public:
  static LobbyPtr join(CSteamID lobby_id, LobbyParams& lobby_params)
  {
    auto const retrieved = retrieveLobby(lobby_id, lobby_params);
    Q_ASSERT(retrieved);
    return LobbyPtr(new Lobby(lobby_id, lobby_params));
  }
  static LobbyPtr create(CSteamID lobby_id, LobbyParams const& lobby_params)
  {
    auto const written = writeLobbyParams(lobby_id, lobby_params);
    Q_ASSERT(written);
    return LobbyPtr(new Lobby(lobby_id, lobby_params));
  }

  Lobby(Lobby const& lobby) = delete;
  void operator=(Lobby const& lobby) = delete;
  ~Lobby()
  {
    auto* steam_matchmaking = SteamMatchmaking();
    if (steam_matchmaking != nullptr)
    {
      steam_matchmaking->LeaveLobby(lobby_id_);
    }
  }

  bool ready() const
  {
    Q_ASSERT(std::all_of(accepted_members_.begin(), accepted_members_.end(), [this](CSteamID steam_id)
    {
      return SteamFriends()->IsUserInSource(steam_id, lobby_id_);
    }));
    auto const result = accepted_members_.size() == player_count_;
    Q_ASSERT(!result || accepted_members_.size() == SteamMatchmaking()->GetNumLobbyMembers(lobby_id_));
    return result;
  }

  void update()
  {
    uint32_t msg_size = 0;
    if (!SteamNetworking()->IsP2PPacketAvailable(&msg_size))
    {
      return;
    }
    Q_ASSERT(msg_size > 0);
    std::string msg(msg_size, 0);
    CSteamID steam_id;
    if (!SteamNetworking()->ReadP2PPacket(&msg.front(), static_cast<uint32_t>(msg.size()), &msg_size, &steam_id))
    {
      return;
    }
    Q_ASSERT(msg.size() == msg_size);
    Q_ASSERT(SteamFriends()->IsUserInSource(steam_id, lobby_id_));
    Q_ASSERT(msg == "hello");
    qDebug() << msg.c_str();
    accepted_members_.insert(steam_id);
  }

private:
  explicit Lobby(CSteamID lobby_id, LobbyParams const& params)
    : lobby_id_(lobby_id)
    , player_count_(params.player_count_)
    , accepted_members_{ SteamUser()->GetSteamID() }
    , on_persona_state_changed_(this, &Lobby::onPersonaStateChanged)
    , on_lobby_data_updated_(this, &Lobby::onLobbyDataUpdated)
    , on_chat_data_updated_(this, &Lobby::onLobbyChatUpdated)
    , on_p2p_session_requested_(this, &Lobby::onP2PSessionRequested)
    , on_p2p_session_connect_failed_(this, &Lobby::onP2PSessionConnectFailed)
  {}

  CSteamID const lobby_id_;
  int const player_count_ = 0;
  QSet<CSteamID> accepted_members_;

  STEAM_CALLBACK(Lobby, onPersonaStateChanged, PersonaStateChange_t, on_persona_state_changed_);
  STEAM_CALLBACK(Lobby, onLobbyDataUpdated, LobbyDataUpdate_t, on_lobby_data_updated_);
  STEAM_CALLBACK(Lobby, onLobbyChatUpdated, LobbyChatUpdate_t, on_chat_data_updated_);
  STEAM_CALLBACK(Lobby, onP2PSessionRequested, P2PSessionRequest_t, on_p2p_session_requested_);
  STEAM_CALLBACK(Lobby, onP2PSessionConnectFailed, P2PSessionConnectFail_t, on_p2p_session_connect_failed_);
};

void SteamNetwork::Lobby::onPersonaStateChanged(PersonaStateChange_t* callback)
{
  if (SteamFriends()->IsUserInSource(callback->m_ulSteamID, lobby_id_))
  {}
}

void SteamNetwork::Lobby::onLobbyDataUpdated(LobbyDataUpdate_t* callback)
{
  if (lobby_id_ != callback->m_ulSteamIDLobby)
  {
    return;
  }
  if (callback->m_ulSteamIDMember == callback->m_ulSteamIDLobby)
  {
//    SteamMatchmaking()->GetLobbyData(callback->m_ulSteamIDLobby, "");
  }
  else
  {
//    SteamMatchmaking()->GetLobbyMemberData(callback->m_ulSteamIDLobby, callback->m_ulSteamIDMember, "");
  }
}

void SteamNetwork::Lobby::onLobbyChatUpdated(LobbyChatUpdate_t* callback)
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
    accepted_members_.insert(id);
    sendP2PMsg(id, "hello");
    break;
  case k_EChatMemberStateChangeLeft:
  default:
    qDebug() << SteamFriends()->GetFriendPersonaName(id) << "left lobby!";
    auto const result = SteamNetworking()->CloseP2PSessionWithUser(id);
    Q_ASSERT(result);
    Q_ASSERT(accepted_members_.contains(id));
    Q_ASSERT(id != SteamUser()->GetSteamID());
    accepted_members_.remove(id);
    break;
  }
}

void SteamNetwork::Lobby::onP2PSessionRequested(P2PSessionRequest_t* callback)
{
  foreachLobbyMember(lobby_id_, [callback, this](CSteamID member)
  {
    if (callback->m_steamIDRemote == member)
    {
      auto const result = SteamNetworking()->AcceptP2PSessionWithUser(member);
      Q_ASSERT(result);
      accepted_members_.insert(member);
    }
  });
}

void SteamNetwork::Lobby::onP2PSessionConnectFailed(P2PSessionConnectFail_t* callback)
{
  qDebug() << SteamFriends()->GetFriendPersonaName(callback->m_steamIDRemote)
           << "connect failed!" << static_cast<int>(callback->m_eP2PSessionError);
}

SteamNetwork::SteamNetwork(QObject* parent, LobbyParams& lobby_params)
  : GameNetwork(parent)
  , callback_timer_id_(startTimer(c_update_time, Qt::TimerType::PreciseTimer))
  , lobby_params_(lobby_params)
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
      SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, lobby_params_.player_count_);
  on_lobby_created_.Set(result, this, &SteamNetwork::onLobbyCreated);
}

void SteamNetwork::joinLobby(LobbyId lobby_id)
{
  if (!SteamFriends()->IsUserInSource(SteamUser()->GetSteamID(), lobby_id))
  {
    auto const result = SteamMatchmaking()->JoinLobby(CSteamID(lobby_id));
    on_lobby_entered_.Set(result, this, &SteamNetwork::onLobbyEntered);
  }
}

void SteamNetwork::timerEvent(QTimerEvent* event)
{
  if (event->timerId() != callback_timer_id_)
  {
    return;
  }
  SteamAPI_RunCallbacks();
  requestLobbies();
  checkLobbyReady();
}

void SteamNetwork::onLobbyListMatched(LobbyMatchList_t* callback, bool)
{
  requesting_lobbies_ = false;
  Lobbies lobbies;

  for (size_t idx = 0; idx < callback->m_nLobbiesMatching; ++idx)
  {
    auto const steam_id = SteamMatchmaking()->GetLobbyByIndex(static_cast<int>(idx));
    LobbyParams params;
    if (retrieveLobby(steam_id, params))
    {
      lobbies.push_back(QVariant::fromValue(params));
    }
  }
  emit lobbiesUpdated(std::move(lobbies));
}

void SteamNetwork::onLobbyCreated(LobbyCreated_t* callback, bool failure)
{
  qDebug() << "onLobbyCreated " << failure;
  if (callback->m_eResult == k_EResultOK)
  {
    lobby_ = Lobby::create(callback->m_ulSteamIDLobby, lobby_params_);
  }
}

void SteamNetwork::onLobbyEntered(LobbyEnter_t* callback, bool failure)
{
  qDebug() << "onLobbyEntered " << failure;
  if (callback->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
  {
    lobby_ = Lobby::join(callback->m_ulSteamIDLobby, lobby_params_);
  }
}

void SteamNetwork::requestLobbies()
{
  if (requesting_lobbies_)
  {
    return;
  }
  requesting_lobbies_ = true;
  auto const result = SteamMatchmaking()->RequestLobbyList();
  on_lobby_list_matched_.Set(result, this, &SteamNetwork::onLobbyListMatched);
}

void SteamNetwork::checkLobbyReady()
{
  if (lobby_ != nullptr && lobby_->ready())
  {
    emit lobbyReady();
  }
}

GameNetworkPtr createGameNetwork(QObject* parent, LobbyParams& lobby_params)
{
  return SteamAPI_RestartAppIfNecessary(480) ? nullptr : new SteamNetwork(parent, lobby_params);
}

} // Network
