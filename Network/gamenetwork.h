#ifndef GAMENETWORK_H
#define GAMENETWORK_H

#include <QVariantList>
#include <QString>
#include <QObject>
#include <QPointer>

#include "../GameLogic/gamemodel.h"

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
  Q_PROPERTY(int initialScores MEMBER initial_scores_)

public:
  LobbyId lobby_id_ = 0;

  QString name_;
  QPoint field_size_;
  int game_speed_ = 0;
  Logic::PlayerId player_count_ = 0;
  Logic::Score initial_scores_ = std::numeric_limits<Logic::Score>::max();
};
using Lobbies = QVariantList;

struct Lobby : public QObject
{
  Q_OBJECT

public:
  using QObject::QObject;

  virtual LobbyParams const& lobbyParams() const = 0;
  virtual Logic::PlayerId currentPlayer() const = 0;

  virtual void initialize(Logic::GameModelPtr game_model) = 0;

signals:
  void ready(LobbyParams const& params);
};
using LobbyPtr = QSharedPointer<Lobby>;

struct GameNetwork : public QObject
{
  Q_OBJECT

public:
  using QObject::QObject;

  virtual void createLobby(LobbyParams const& params) = 0;
  virtual void joinLobby(LobbyParams const& params) = 0;

signals:
  void lobbyCreated(LobbyPtr lobby);
  void lobbiesUpdated(Lobbies lobbies);
};
using GameNetworkPtr = QPointer<GameNetwork>;
inline GameNetworkPtr createGameNetwork(QObject*)
{
  throw std::runtime_error("Game network initialization failed!");
}

inline bool operator == (LobbyParams const& lhs, LobbyParams const& rhs)
{
  return lhs.name_ == rhs.name_
      && lhs.field_size_ == rhs.field_size_
      && lhs.game_speed_ == rhs.game_speed_
      && lhs.player_count_ == rhs.player_count_
      && lhs.initial_scores_ == rhs.initial_scores_;
}

} // Network

Q_DECLARE_METATYPE(Network::LobbyParams)

#endif // GAMENETWORK_H
