#ifndef STEAMNETWORK_H
#define STEAMNETWORK_H

#include <set>

#include <steam/steam_api.h>

#include "../gamenetwork.h"

namespace Network {

class SteamLobby final : public Lobby
{
  Q_OBJECT

public:
  static LobbyPtr join(LobbyParams const& params);
  static LobbyPtr create(LobbyParams const& params);

  SteamLobby(SteamLobby&& lobby) = default;
  SteamLobby& operator = (SteamLobby&& lobby) = default;

  SteamLobby(SteamLobby const& lobby) = delete;
  SteamLobby& operator = (SteamLobby const& lobby) = delete;
  ~SteamLobby() override;

  LobbyParams const& lobbyParams() const override { return lobby_params_; }
  Logic::PlayerId currentPlayer() const override { return current_player_; }

  void initialize(Logic::GameModelPtr game_model) override;

private:
  explicit SteamLobby(LobbyParams params);

  CSteamID lobbyId() const;
  bool isReady() const;
  bool initialized() const { return game_model_ != nullptr; }

  void timerEvent(QTimerEvent* event) override;

  int const callback_timer_id_ = 0;
  LobbyParams const lobby_params_;
  std::set<CSteamID> accepted_members_;
  Logic::GameModelPtr game_model_;
  Logic::PlayerId current_player_ = Logic::c_max_player_count;

  STEAM_CALLBACK(SteamLobby, onPersonaStateChanged, PersonaStateChange_t, on_persona_state_changed_);
  STEAM_CALLBACK(SteamLobby, onLobbyDataUpdated, LobbyDataUpdate_t, on_lobby_data_updated_);
  STEAM_CALLBACK(SteamLobby, onLobbyChatUpdated, LobbyChatUpdate_t, on_chat_data_updated_);
  STEAM_CALLBACK(SteamLobby, onP2PSessionRequested, P2PSessionRequest_t, on_p2p_session_requested_);
  STEAM_CALLBACK(SteamLobby, onP2PSessionConnectFailed, P2PSessionConnectFail_t, on_p2p_session_connect_failed_);
};

class SteamNetwork final : public GameNetwork
{
  Q_OBJECT

public:
  explicit SteamNetwork(QObject* parent);
  ~SteamNetwork() override;

  void createLobby(LobbyParams const& params) override;
  void joinLobby(LobbyParams const& params) override;

protected:
  void timerEvent(QTimerEvent* event) override;

private:
  void onLobbyListMatched(LobbyMatchList_t* callback, bool failure);
  void onLobbyCreated(LobbyCreated_t* callback, bool failure);
  void onLobbyEntered(LobbyEnter_t* callback, bool failure);
  void requestLobbies();

  int const callback_timer_id_ = 0;
  bool requesting_lobbies_ = false;
  LobbyParams lobby_params_;

  CCallResult<SteamNetwork, LobbyMatchList_t> on_lobby_list_matched_;
  CCallResult<SteamNetwork, LobbyCreated_t> on_lobby_created_;
  CCallResult<SteamNetwork, LobbyEnter_t> on_lobby_entered_;
};

} // Network

inline uint qHash(CSteamID steam_id, uint seed)
{
  return ::qHash(steam_id.ConvertToUint64(), seed);
}

#endif // STEAMNETWORK_H
