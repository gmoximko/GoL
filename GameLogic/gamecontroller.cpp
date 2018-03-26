#include "gamecontroller.h"

namespace Logic {

namespace {

class GameControllerImpl : public GameController
{
public:
  explicit GameControllerImpl(Params const& params)
    : game_model_(params.game_model_)
  {}

  void addPattern(PatternTrs pattern_trs) override
  {
    auto const& pattern = pattern_trs.first;
    auto const& trs = pattern_trs.second;
    for (auto const& unit : pattern->points())
    {
      game_model_->addUnit(unit * trs);
    }
  }

private:
  GameModelMutablePtr game_model_;
};

} // namespace

GameControllerPtr createGameController(GameController::Params const& params)
{
  return std::make_unique<GameControllerImpl>(params);
}

} // Logic
