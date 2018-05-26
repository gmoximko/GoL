#include <QDebug>
#include <QTimerEvent>

#include "steamnetwork.h"

namespace Network {

namespace {

constexpr auto const* c_lobby_name = "name";
constexpr auto const* c_rich_status = "status";

} // namespace

SteamNetwork::SteamNetwork(QObject* parent, Params const& params)
  : GameNetwork(parent)
  , params_(params)
  , callback_timer_id_(startTimer(params_.callback_interval_, Qt::TimerType::PreciseTimer))
{
  Q_ASSERT(callback_timer_id_ != 0);
  if (!SteamAPI_Init())
  {
    throw std::runtime_error("Steam initialization failed!");
  }
  qDebug() << SteamFriends()->GetPersonaName();
}

SteamNetwork::~SteamNetwork()
{
  SteamAPI_Shutdown();
}

void SteamNetwork::createLobby()
{
  auto const result = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, params_.max_player_count_);
  lobby_created_callback_.Set(result, this, &SteamNetwork::onLobbyCreated);
  SteamFriends()->SetRichPresence(c_rich_status, "Creating a lobby");
}

void SteamNetwork::joinLobby(LobbyId lobby_id)
{
  auto const result = SteamMatchmaking()->JoinLobby(CSteamID(lobby_id));
  lobby_enter_callback_.Set(result, this, &SteamNetwork::onLobbyEntered);
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
  lobby_match_callback_.Set(result, this, &SteamNetwork::onLobbyListMatched);
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
      lobbies.push_back(QVariant::fromValue(Lobby(steam_id.ConvertToUint64(), QString(lobby_name))));
    }
  }
  emit lobbiesUpdated(std::move(lobbies));
}

void SteamNetwork::onLobbyCreated(LobbyCreated_t* callback, bool failure)
{
  if (callback->m_eResult == k_EResultOK)
  {
    auto const* persona_name = SteamFriends()->GetPersonaName();
    SteamMatchmaking()->SetLobbyData(callback->m_ulSteamIDLobby, c_lobby_name, persona_name);
  }
  qDebug() << "onLobbyCreated " << failure;
}

void SteamNetwork::onLobbyEntered(LobbyEnter_t* callback, bool failure)
{
  qDebug() << "onLobbyEntered " << failure;
}

GameNetworkPtr createGameNetwork(QObject* parent, GameNetwork::Params const& params)
{
  return SteamAPI_RestartAppIfNecessary(480)
      ? nullptr
      : QPointer<GameNetwork>(new SteamNetwork(parent, params));
}

} // Network
