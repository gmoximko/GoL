#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQuickItem>

#include "../GameLogic/gamemodel.h"
#include "../GameLogic/gamecontroller.h"
#include "../GameLogic/gameparameters.h"
#include "../Network/gamenetwork.h"
#include "gameview.h"

namespace View {

class MainWindow : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(Logic::GameParamsQmlAdaptor* gameParams READ gameParams CONSTANT)

public:
  explicit MainWindow(QQuickItem* parent = nullptr);
  Logic::GameParamsQmlAdaptor* gameParams() { return game_params_.data(); }

  Q_INVOKABLE QQuickItem* createGame();
  Q_INVOKABLE bool destroyGame();
  Q_INVOKABLE void createLobby();
  Q_INVOKABLE void joinLobby(Logic::LobbyId lobby_id);

private:
  void createGameModel();
  void createGameController();
  void createGameView();

  Logic::GameModelMutablePtr game_model_;
  Logic::GameControllerPtr game_controller_;
  Logic::GameParamsPtr game_params_;
  View::GameViewPtr game_view_;
  QScopedPointer<QQuickItem> game_window_;
  Network::GameNetworkPtr game_network_;
};

}

#endif // MAINWINDOW_H
