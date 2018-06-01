#ifndef GAMENETWORK_H
#define GAMENETWORK_H

#include <memory>

#include <QPoint>
#include <QVariantList>
#include <QString>
#include <QObject>
#include <QPointer>

#include "GameLogic/gameparameters.h"

namespace Network {

using Lobbies = QVariantList;

class GameNetwork : public QObject
{
  Q_OBJECT

public:
  using QObject::QObject;

  virtual void createLobby() = 0;
  virtual void joinLobby(Logic::LobbyId lobby_id) = 0;

signals:
  void lobbiesUpdated(Lobbies lobbies);
};
using GameNetworkPtr = QPointer<GameNetwork>;
GameNetworkPtr createGameNetwork(QObject* parent, Logic::GameParamsPtr game_params);

} // Network

#endif // GAMENETWORK_H
