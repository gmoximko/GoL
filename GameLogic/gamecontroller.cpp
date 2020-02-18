#include <cmath>

#include <QTimerEvent>
#include <QDebug>

#include "gamecontroller.h"

namespace Logic {

GameController::GameController(QObject* parent, Params const& params)
  : QObject(parent)
  , update_time_(params.update_time_)
  , step_timer_id_(startTimer(update_time_, Qt::TimerType::PreciseTimer))
  , game_model_(params.game_model_)
{
  qDebug() << "GameController(" << update_time_ << ')';
  Q_ASSERT(step_timer_id_ != 0);
  Q_ASSERT(game_model_ != nullptr);
}

GameController::GameController(QObject* parent,
                               GameModelMutablePtr game_model,
                               SavedData const& data)
  : QObject(parent)
  , update_time_(data["updateTime"].toInt())
  , step_timer_id_(startTimer(update_time_, Qt::TimerType::PreciseTimer))
  , game_model_(game_model)
  , stopped_(data["stopped"].toBool())
{
  qDebug() << "GameController(" << update_time_ << ')';
  Q_ASSERT(game_model_ != nullptr);

  if (step_timer_id_ == 0)
  {
    throw std::runtime_error("updateTime is greater than max or less than a min!");
  }
}

GameController::~GameController()
{
  qDebug() << "~GameController()";
}

void GameController::onApplicationActive()
{
  if (stopped_due_inactive_)
  {
    Q_ASSERT(stopped_);
    stopped_due_inactive_ = false;
    onStop();
    Q_ASSERT(!stopped_);
  }
}

void GameController::onApplicationInactive()
{
  if (!stopped_)
  {
    Q_ASSERT(!stopped_due_inactive_);
    stopped_due_inactive_ = true;
    onStop();
    Q_ASSERT(stopped_);
  }
}

Serializable::SavedData GameController::serialize() const
{
  SavedData data;
  data["updateTime"] = update_time_;
  data["stopped"] = stopped_;
  return data;
}

void GameController::addPattern(PatternTrs pattern_trs)
{
  auto const& pattern = pattern_trs.first;
  auto const& trs = pattern_trs.second;
  Q_ASSERT(pattern != nullptr);
  LifeUnits units;
  units.reserve(pattern->points().size());
  for (auto const& unit : pattern->points())
  {
    auto const position = loopPos(unit * trs, game_model_->cells());
    units.push_back(LifeUnit(static_cast<uint16_t>(position.x()),
                             static_cast<uint16_t>(position.y())));
  }
  Q_ASSERT(!units.empty());
  game_model_->lifeProcessor().addUnits(std::move(units));
}

bool GameController::onStop()
{
  stopped_ = !stopped_;
  return stopped_;
}

void GameController::onGameSpeedChanged(int update_time)
{
  auto const step_timer_id = startTimer(update_time);
  if (step_timer_id != 0)
  {
    killTimer(step_timer_id_);
    step_timer_id_ = step_timer_id;
    update_time_ = update_time;
  }
  else
  {
    Q_UNREACHABLE();
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
  game_model_->lifeProcessor().processLife(!stopped_);
  emit stepMade(scores_);
}

} // Logic
