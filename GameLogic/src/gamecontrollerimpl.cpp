#include "gamecontrollerimpl.h"

namespace Logic {

GameControllerImpl::GameControllerImpl(QObject* parent, Params const& params)
  : GameController(parent)
  , game_model_(params.game_model_)
  , timer_(this)
{
  QObject::connect(&timer_, &QTimer::timeout, this, &GameControllerImpl::makeStep);
  timer_.start(params.update_time_);
}

void GameControllerImpl::addPattern(PatternTrs pattern_trs)
{
  auto const& pattern = pattern_trs.first;
  auto const& trs = pattern_trs.second;
  Q_ASSERT(pattern != nullptr);
  for (auto const& unit : pattern->points())
  {
    game_model_->addUnit(unit * trs);
  }
}

void GameControllerImpl::makeStep()
{
  game_model_->makeStep();
  emit onStepMade();
}

GameControllerPtr createGameController(QObject* parent, GameController::Params const& params)
{
  return GameControllerPtr(new GameControllerImpl(parent, params));
}

} // Logic
