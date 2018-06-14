#ifndef STEAMNETWORK_H
#define STEAMNETWORK_H

#include <steam/steam_api.h>

#include "../gamenetwork.h"

namespace Network {

class SteamNetwork : public GameNetwork
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
