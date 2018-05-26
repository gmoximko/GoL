#ifndef GAMENETWORK_H
#define GAMENETWORK_H

#include <memory>

#include <QVariantList>
#include <QString>
#include <QObject>
#include <QPointer>

namespace Network {

using LobbyId = uint64_t;
class Lobby
{
  Q_GADGET
  Q_PROPERTY(uint64_t lobbyId READ lobbyId CONSTANT)
  Q_PROPERTY(QString name READ name CONSTANT)

public:
  Lobby() = default;
  explicit Lobby(LobbyId lobby_id, QString name)
    : lobby_id_(lobby_id)
    , name_(std::move(name))
  {}

  LobbyId lobbyId() const
  {
    return lobby_id_;
  }
  QString name() const
  {
    return name_;
  }

private:
  LobbyId const lobby_id_ = 0;
  QString const name_;
};
using Lobbies = QVariantList;

class GameNetwork : public QObject
{
  Q_OBJECT

public:
  struct Params
  {
    int callback_interval_ = 30;
    int max_player_count_ = 4;
  };

  using QObject::QObject;

  virtual void createLobby() = 0;
  virtual void joinLobby(LobbyId lobby_id) = 0;

signals:
  void lobbiesUpdated(Lobbies lobbies);
};
using GameNetworkPtr = QPointer<GameNetwork>;
GameNetworkPtr createGameNetwork(QObject* parent, GameNetwork::Params const& params);

} // Network

Q_DECLARE_METATYPE(Network::Lobby)

#endif // GAMENETWORK_H
