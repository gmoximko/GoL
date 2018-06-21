#include <QTimerEvent>

#include "gamecontroller.h"

namespace Logic {

GameController::GameController(QObject* parent, Params const& params)
  : QObject(parent)
  , step_timer_id_(startTimer(params.update_time_, Qt::TimerType::PreciseTimer))
  , current_player_(params.current_player_)
  , game_model_(params.game_model_)
{
  Q_ASSERT(step_timer_id_ != 0);
}

void GameController::addPattern(PatternTrs pattern_trs)
{
  auto const& pattern = pattern_trs.first;
  auto const& trs = pattern_trs.second;
  Q_ASSERT(pattern != nullptr);
  for (auto const& unit : pattern->points())
  {
    game_model_->addUnit(unit * trs, current_player_);
  }
}

void GameController::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == step_timer_id_)
  {
    makeStep();
  }
}

void GameController::makeStep()
{
  game_model_->makeStep();
  emit stepMade();
}

} // Logic
