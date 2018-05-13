#include <QTimerEvent>

#include "gamecontrollerimpl.h"

namespace Logic {

TimerController::TimerController(QObject* parent, Params const& params)
  : GameController(parent)
  , step_timer_id_(startTimer(params.update_time_, Qt::TimerType::PreciseTimer))
  , game_model_(params.game_model_)
{
  Q_ASSERT(step_timer_id_ != 0);
}

void TimerController::addPattern(PatternTrs pattern_trs)
{
  auto const& pattern = pattern_trs.first;
  auto const& trs = pattern_trs.second;
  Q_ASSERT(pattern != nullptr);
  for (auto const& unit : pattern->points())
  {
    game_model_->addUnit(unit * trs, 0);
  }
}

void TimerController::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == step_timer_id_)
  {
    makeStep();
  }
}

void TimerController::makeStep()
{
  game_model_->makeStep();
  emit stepMade();
}

GameControllerPtr createGameController(QObject* parent, GameController::Params const& params)
{
  return GameControllerPtr(new TimerController(parent, params));
}

} // Logic
