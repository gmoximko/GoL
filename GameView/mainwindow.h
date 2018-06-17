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
  ~GameParams() override
  {
    qDebug() << "~GameParams()";
  }

  auto lobby() const { return lobby_; }
  auto const& getParams() const { return game_params_; }
  Q_INVOKABLE void setParams(QVariant const& lobby_params)
  {
    game_params_ = lobby_params.value<Network::LobbyParams>();
  }

  auto lobbyId() const { return game_params_.lobby_id_; }
  auto name() const { return game_params_.name_; }
  auto fieldSize() const { return game_params_.field_size_; }
  auto gameSpeed() const { return game_params_.game_speed_; }
  auto playerCount() const { return game_params_.player_count_; }

  void setName(QString name) { game_params_.name_ = std::move(name); }
  void setFieldSize(QPoint field_size) { game_params_.field_size_ = field_size; }
  void setGameSpeed(int game_speed) { game_params_.game_speed_ = game_speed; }
  void setPlayerCount(int player_count)
  {
    game_params_.player_count_ = static_cast<Logic::PlayerId>(player_count);
  }

public slots:
  void setLobby(Network::LobbyPtr lobby);

private slots:
  void ready(Network::LobbyParams const& lobby_params)
  {
    Q_ASSERT(lobby_params == game_params_);
    emit start(this);
  }

signals:
  void start(GameParams* params);

private:
  Network::LobbyPtr lobby_;
  Network::LobbyParams game_params_;
};

class MainWindow : public QQuickItem
{
  Q_OBJECT

public:
  explicit MainWindow(QQuickItem* parent = nullptr);

  Q_INVOKABLE bool isNetworkEnabled() const { return game_network_ != nullptr; }

  Q_INVOKABLE QQuickItem* createGame(GameParams* params);
  Q_INVOKABLE bool destroyGame();
  Q_INVOKABLE void createLobby(GameParams* params);
  Q_INVOKABLE void joinLobby(GameParams* params);

private:
  void createGameModel(GameParams const& params);
  void createGameController(GameParams const& params);
  void createGameView(GameParams const& params);

  Logic::GameModelMutablePtr game_model_;
  Logic::GameControllerPtr game_controller_;
  QScopedPointer<QQuickItem> game_window_;
  Network::GameNetworkPtr game_network_;
  GameViewPtr game_view_;
};

}

#endif // MAINWINDOW_H
