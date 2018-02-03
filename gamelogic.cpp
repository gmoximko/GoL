#include "gamelogic.h"

namespace Logic {

namespace {

class GameFieldImpl : public GameField
{
public:
  explicit GameFieldImpl(Params const& params)
    : cells_(params.cells_)
  {}

  QPoint cells() const override { return cells_; }

private:
  QPoint const cells_;
};

} // namespace

GameFieldPtr createGameField(GameField::Params const& params)
{
  return std::make_unique<GameFieldImpl>(params);
}

} // Logic
