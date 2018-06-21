#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QPointer>

#include "gamemodel.h"

namespace Logic {

struct GameController : public QObject
{
  Q_OBJECT

public:
  struct Params
  {
    GameModelMutablePtr game_model_;
    int update_time_ = 0;
    PlayerId current_player_ = 0;
  };

  explicit GameController(QObject* parent, Params const& params);

public slots:
  virtual void addPattern(PatternTrs pattern_trs);

signals:
  void stepMade();

protected:
  void timerEvent(QTimerEvent* event) override;

private:
  void makeStep();

  int const step_timer_id_ = 0;
  PlayerId const current_player_ = 0;
  GameModelMutablePtr game_model_;
};
using GameControllerPtr = QPointer<GameController>;

} // Logic

#endif // GAMECONTROLLER_H
