#ifndef GAMENETWORK_H
#define GAMENETWORK_H

#include <memory>

#include <QPoint>
#include <QVariantList>
#include <QString>
#include <QObject>
#include <QPointer>

namespace Network {

using LobbyId = uint64_t;
struct LobbyParams
{
  Q_GADGET
  Q_PROPERTY(uint64_t lobbyId MEMBER lobby_id_)
  Q_PROPERTY(QString name MEMBER name_)
  Q_PROPERTY(QPoint fieldSize MEMBER field_size_)
  Q_PROPERTY(int gameSpeed MEMBER game_speed_)
  Q_PROPERTY(int playerCount MEMBER player_count_)

public:
  LobbyId lobby_id_ = 0;
  QString name_;

  QPoint field_size_;
  int game_speed_ = 0;
  int player_count_ = 0;
};
using Lobbies = QVariantList;

class GameNetwork : public QObject
{
  Q_OBJECT

public:
  using QObject::QObject;

  virtual void createLobby() = 0;
  virtual void joinLobby(LobbyId lobby_id) = 0;

signals:
  void lobbyReady();
  void lobbiesUpdated(Lobbies lobbies);
};
using GameNetworkPtr = QPointer<GameNetwork>;
GameNetworkPtr createGameNetwork(QObject* parent, LobbyParams& lobby_params);

} // Network

Q_DECLARE_METATYPE(Network::LobbyParams)

#endif // GAMENETWORK_H
