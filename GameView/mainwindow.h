#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQuickItem>

#include "../GameLogic/gamemodel.h"
#include "../GameLogic/gamecontroller.h"
#include "../Network/gamenetwork.h"
#include "gameview.h"

namespace View {

class GameParams : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(QPoint fieldSize READ fieldSize WRITE setFieldSize)
  Q_PROPERTY(int gameSpeed READ gameSpeed WRITE setGameSpeed)
  Q_PROPERTY(int playerCount READ playerCount WRITE setPlayerCount)

public:
  using QObject::QObject;

  QString name() const { return game_params_.name_; }
  QPoint fieldSize() const { return game_params_.field_size_; }
  int gameSpeed() const { return game_params_.game_speed_; }
  int playerCount() const { return game_params_.player_count_; }

  Network::LobbyParams& getParams() { return game_params_; }

  void setName(QString name) { game_params_.name_ = std::move(name); }
  void setFieldSize(QPoint field_size) { game_params_.field_size_ = field_size; }
  void setGameSpeed(int game_speed) { game_params_.game_speed_ = game_speed; }
  void setPlayerCount(int player_count)
  {
    game_params_.player_count_ = static_cast<Logic::PlayerId>(player_count);
  }

private:
  Network::LobbyParams game_params_;
};
using GameParamsPtr = QPointer<GameParams>;

class MainWindow : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(View::GameParams* gameParams READ gameParams CONSTANT)

public:
  explicit MainWindow(QQuickItem* parent = nullptr);
  GameParams* gameParams() { return game_params_.data(); }

  Q_INVOKABLE bool isNetworkEnabled() const { return game_network_ != nullptr; }

  Q_INVOKABLE QQuickItem* createGame();
  Q_INVOKABLE bool destroyGame();
  Q_INVOKABLE void createLobby();
  Q_INVOKABLE void joinLobby(QVariant const& lobby);

private:
  void createGameModel();
  void createGameController();
  void createGameView();

  Logic::GameModelMutablePtr game_model_;
  Logic::GameControllerPtr game_controller_;
  QScopedPointer<QQuickItem> game_window_;
  Network::GameNetworkPtr game_network_;

  GameParamsPtr game_params_;
  GameViewPtr game_view_;
};

}

#endif // MAINWINDOW_H
