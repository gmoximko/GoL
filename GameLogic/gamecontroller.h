#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QPointer>

#include "gamemodel.h"

namespace Logic {

class GameController final : public QObject
{
  Q_OBJECT

public:
  struct Params
  {
    GameModelMutablePtr game_model_;
    int update_time_ = 0;
    Score initial_scores_ = 0;
    PlayerId current_player_ = 0;
  };

  explicit GameController(QObject* parent, Params const& params);
  ~GameController() override;

public slots:
  bool addPattern(PatternTrs pattern_trs);
  bool onStop();

signals:
  void stepMade(Score scores);

protected:
  void timerEvent(QTimerEvent* event) override;

private:
  class Command;
  using StepId = uint64_t;

  void makeStep();
  void applyCommands();
  void updateStep();

  int const step_timer_id_ = 0;
  PlayerId const player_ = 0;
  Score const score_addition_ = 0;

  GameModelMutablePtr game_model_;
  std::vector<Command> commands_;
  StepId step_ = 0;
  Score scores_ = 0;
  uint64_t average_computation_duration_ = 0;
  bool stopped_ = false;
};
using GameControllerPtr = QPointer<GameController>;

} // Logic

#endif // GAMECONTROLLER_H
