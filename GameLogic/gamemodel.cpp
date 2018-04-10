#include <QHash>
#include <QDebug>

#include "../Utilities/qtutilities.h"
#include "../Utilities/rleparser.h"
#include "src/patterns.h"
#include "src/lifeprocessor.h"
#include "src/gpulifeprocessor.h"
#include "gamemodel.h"

namespace Logic {

namespace {

template<class PatternsStrategy = AccumulatePatterns,
         class LifeProcessorStrategy = SimpleLifeProcessor>
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
  SizeT patternCount() const override
  {
    return all_patterns_.patternCount();
  }
  PatternPtr patternAt(SizeT idx) const override
  {
    return all_patterns_.patternAt(idx);
  }
  LifeUnits const& lifeUnits() const override
  {
    return life_processor_.lifeUnits();
  }

  void addUnit(LifeUnit const& life_unit) override
  {
    life_processor_.addUnit(life_unit);
  }
  void makeStep() override
  {
    life_processor_.processLife();
  }

private:
  QPoint const cells_;
  PatternsStrategy const all_patterns_;
  LifeProcessorStrategy life_processor_;
};

} // namespace

GameModelMutablePtr createGameModel(GameModel::Params const& params)
{
  return Utilities::Qt::makeShared<GameModelImpl<>>(params);
}

} // Logic
