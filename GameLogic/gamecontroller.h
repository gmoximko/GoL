#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QMatrix>

#include "gamemodel.h"

namespace Logic {

struct GameController
{
  struct Params
  {
    GameModelMutablePtr game_model_;
  };

  virtual ~GameController() = default;
  virtual void addPattern(PatternTrs pattern_trs) = 0;
};
using GameControllerPtr = std::unique_ptr<GameController>;
GameControllerPtr createGameController(GameController::Params const& params);

} // Logic

#endif // GAMECONTROLLER_H
