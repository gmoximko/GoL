#include "gamemodel.h"
#include "../Utilities/qtutilities.h"
#include "../Utilities/rleparser.h"

namespace Logic {

namespace {

class GameModelImpl : public GameModel
{
public:
  explicit GameModelImpl(Params const& params)
    : cells_(params.cells_)
    , all_patterns_(Utilities::createPatterns())
  {
    //#if defined(QT_DEBUG)
    //  for (Logic::SizeT idx = 0; idx < all_patterns_->patternCount(); ++idx)
    //  {
    //    auto const pattern = all_patterns_->patternAt(idx);
    //    Q_ASSERT(!pattern->name().isEmpty());
    //    Q_ASSERT(!pattern->points().isEmpty());
    //    Q_ASSERT(pattern->scores() > 0);
    //    Q_ASSERT(pattern->size() != QPoint());
    //  }
    //#endif
  }

  QPoint cells() const override
  {
    return cells_;
  }
  PatternsPtr const& allPatterns() const override
  {
    return all_patterns_;
  }
  LifeUnits const& lifeUnits() const override
  {
    return life_units_;
  }

  void addUnit(LifeUnit const& life_unit) override
  {
    life_units_.push_back(life_unit);
  }

private:
  QPoint const cells_;
  Logic::PatternsPtr const all_patterns_;

  LifeUnits life_units_;
};

} // namespace

GameModelMutablePtr createGameModel(GameModel::Params const& params)
{
  return Utilities::Qt::makeShared<GameModelImpl>(params);
}

} // Logic
