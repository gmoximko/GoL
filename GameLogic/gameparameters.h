#ifndef GAMEPARAMETERS_H
#define GAMEPARAMETERS_H

#include <QtQml>
#include <QDebug>

#include "gamemodel.h"

namespace Logic {

using LobbyId = uint64_t;
struct GameParams
{
  Q_GADGET
  Q_PROPERTY(uint64_t lobbyId MEMBER lobby_id_)
  Q_PROPERTY(QString name MEMBER name_)

public:
  LobbyId lobby_id_ = 0;
  QString name_;

  QPoint field_size_;
  int game_speed_ = 100;
  PlayerId max_player_count_ = 0;
};

class GameParamsQmlAdaptor : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QPoint fieldSize READ fieldSize WRITE setFieldSize)
  Q_PROPERTY(int gameSpeed READ gameSpeed WRITE setGameSpeed)
  Q_PROPERTY(int maxPlayerCount READ maxPlayerCount WRITE setMaxPlayerCount)

public:
  using QObject::QObject;

  QPoint fieldSize() const { return game_params_.field_size_; }
  int gameSpeed() const { return game_params_.game_speed_; }
  int maxPlayerCount() const { return game_params_.max_player_count_; }

  void setFieldSize(QPoint field_size) { game_params_.field_size_ = field_size; }
  void setGameSpeed(int game_speed) { game_params_.game_speed_ = game_speed; }
  void setMaxPlayerCount(int max_player_count)
  {
    game_params_.max_player_count_ = static_cast<Logic::PlayerId>(max_player_count);
  }

  void setParams(GameParams params)
  {
    game_params_ = std::move(params);
  }

private:
  GameParams game_params_;
};
using GameParamsPtr = QPointer<GameParamsQmlAdaptor>;

} // Logic

Q_DECLARE_METATYPE(Logic::GameParams)

#endif // GAMEPARAMETERS_H
