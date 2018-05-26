#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQuickItem>

#include "../GameLogic/gamemodel.h"
#include "../GameLogic/gamecontroller.h"
#include "../Network/gamenetwork.h"
#include "gameview.h"

namespace View {

class MainWindow : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QPoint cells MEMBER cells_)

public:
  explicit MainWindow(QQuickItem* parent = nullptr);
  Q_INVOKABLE QQuickItem* createGame();
  Q_INVOKABLE bool destroyGame();
  Q_INVOKABLE void createLobby()
  {
    game_network_->createLobby();
  }
  Q_INVOKABLE void joinLobby(Network::LobbyId lobby_id)
  {
    game_network_->joinLobby(lobby_id);
  }

private:
  void createGameModel();
  void createGameController();
  void createGameView();

  Logic::GameModelMutablePtr game_model_;
  Logic::GameControllerPtr game_controller_;
  View::GameViewPtr game_view_;
  QScopedPointer<QQuickItem> game_window_;
  Network::GameNetworkPtr game_network_;

  // Game parameters
  QPoint cells_;
};

}

#endif // MAINWINDOW_H
