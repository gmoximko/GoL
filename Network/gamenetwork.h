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

struct Lobby
{
  virtual ~Lobby() = default;
  virtual bool ready() const = 0;
};
using LobbyPtr = QSharedPointer<Lobby>;

class GameNetwork : public QObject
{
  Q_OBJECT

public:
  using QObject::QObject;

  virtual void createLobby(LobbyParams const& params) = 0;
  virtual void joinLobby(LobbyParams const& params) = 0;

signals:
  void lobbyCreated(LobbyPtr);
  void lobbiesUpdated(Lobbies);
};
using GameNetworkPtr = QPointer<GameNetwork>;
GameNetworkPtr createGameNetwork(QObject* parent);

inline bool operator == (LobbyParams const& lhs, LobbyParams const& rhs)
{
  return lhs.lobby_id_ == rhs.lobby_id_
      && lhs.name_ == rhs.name_
      && lhs.field_size_ == rhs.field_size_
      && lhs.game_speed_ == rhs.game_speed_
      && lhs.player_count_ == rhs.player_count_;
}

} // Network

Q_DECLARE_METATYPE(Network::LobbyParams)

#endif // GAMENETWORK_H
