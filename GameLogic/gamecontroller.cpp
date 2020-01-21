#include <cmath>

#include <QTimerEvent>

#include "gamecontroller.h"

namespace Logic {

class GameController::Command
{
public:
  explicit Command(StepId step, PatternTrs pattern_trs, QPoint cells, PlayerId player)
    : step_(step)
    , player_(player)
    , pattern_trs_(std::move(pattern_trs))
    , cells_(cells)
  {}

  void apply(LifeProcessor& processor) const
  {
    auto const& pattern = pattern_trs_.first;
    auto const& trs = pattern_trs_.second;
    Q_ASSERT(pattern != nullptr);
    LifeUnits units;
    units.reserve(pattern->points().size());
    for (auto const& unit : pattern->points())
    {
      auto const position = loopPos(unit * trs, cells_);
      units.push_back(LifeUnit(static_cast<uint16_t>(position.x()),
                               static_cast<uint16_t>(position.y())));
    }
    Q_ASSERT(!units.empty());
    processor.addUnits(std::move(units));
  }

private:
  StepId const step_ = 0;
  PlayerId const player_ = 0;
  PatternTrs const pattern_trs_;
  QPoint const cells_;
};

GameController::GameController(QObject* parent, Params const& params)
  : QObject(parent)
  , step_timer_id_(startTimer(params.update_time_, Qt::TimerType::PreciseTimer))
  , player_(params.current_player_)
  , score_addition_(params.initial_scores_)
  , game_model_(params.game_model_)
  , scores_(params.initial_scores_)
{
  Q_ASSERT(scores_ > 0);
  Q_ASSERT(step_timer_id_ != 0);
  Q_ASSERT(player_ >= 0 && player_ < c_max_player_count);
  Q_ASSERT(game_model_ != nullptr);
}

GameController::~GameController() = default;

bool GameController::addPattern(PatternTrs pattern_trs)
{
  auto const pattern_scores = pattern_trs.first->scores();
  auto const result = pattern_scores <= scores_;
  if (result)
  {
//    scores_ -= pattern_scores;
    commands_.emplace_back(step_, std::move(pattern_trs), game_model_->cells(), player_);
    if (game_model_->lifeProcessor().computed())
    {
      applyCommands();
    }
  }
  return result;
}

bool GameController::onStop()
{
  stopped_ = !stopped_;
  return stopped_;
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
  auto& life_processor = game_model_->lifeProcessor();
  if (!life_processor.computed())
  {
    return;
  }
  applyCommands();
  life_processor.processLife(!stopped_);
  updateStep();
  emit stepMade(scores_);
}

void GameController::applyCommands()
{
  auto& life_processor = game_model_->lifeProcessor();
  Q_ASSERT(life_processor.computed());
  for (auto const& command : commands_)
  {
    command.apply(life_processor);
  }
  commands_.clear();
}

void GameController::updateStep()
{
  if (!stopped_)
  {
    ++step_;
  }
}

} // Logic
