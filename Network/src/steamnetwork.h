#ifndef STEAMNETWORK_H
#define STEAMNETWORK_H

#include <steam/steam_api.h>

#include "../gamenetwork.h"

namespace Network {

class SteamNetwork : public GameNetwork
{
  Q_OBJECT

public:
  explicit SteamNetwork(QObject* parent, Params const& params);
  ~SteamNetwork() override;

  void createLobby() override;
  void joinLobby(LobbyId lobby_id) override;

protected:
  void timerEvent(QTimerEvent* event) override;

private:
  void onLobbyListMatched(LobbyMatchList_t* callback, bool failure);
  void onLobbyCreated(LobbyCreated_t* callback, bool failure);
  void onLobbyEntered(LobbyEnter_t* callback, bool failure);

  Params const params_;
  int const callback_timer_id_ = 0;

  CCallResult<SteamNetwork, LobbyMatchList_t> lobby_match_callback_;
  CCallResult<SteamNetwork, LobbyCreated_t> lobby_created_callback_;
  CCallResult<SteamNetwork, LobbyEnter_t> lobby_enter_callback_;

  bool requesting_lobbies_ = false;
};

} // Network

#endif // STEAMNETWORK_H
