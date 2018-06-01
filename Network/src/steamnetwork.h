#ifndef STEAMNETWORK_H
#define STEAMNETWORK_H

#include <steam/steam_api.h>

#include "../gamenetwork.h"

namespace Network {

class SteamNetwork : public GameNetwork
{
  Q_OBJECT

public:
  explicit SteamNetwork(QObject* parent, Logic::GameParamsPtr game_params);
  ~SteamNetwork() override;

  void createLobby() override;
  void joinLobby(Logic::LobbyId lobby_id) override;

protected:
  void timerEvent(QTimerEvent* event) override;

private:
  void onLobbyListMatched(LobbyMatchList_t* callback, bool failure);
  void onLobbyCreated(LobbyCreated_t* callback, bool failure);
  void onLobbyEntered(LobbyEnter_t* callback, bool failure);

  int const callback_timer_id_ = 0;
  Logic::GameParamsPtr game_params_;

  CCallResult<SteamNetwork, LobbyMatchList_t> on_lobby_list_matched_;
  CCallResult<SteamNetwork, LobbyCreated_t> on_lobby_created_;
  CCallResult<SteamNetwork, LobbyEnter_t> on_lobby_entered_;

  bool requesting_lobbies_ = false;
};

} // Network

#endif // STEAMNETWORK_H
