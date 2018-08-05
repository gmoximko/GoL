#include <QHash>
#include <QDebug>

#include "../Utilities/qtutilities.h"
#include "../Utilities/rleparser.h"
#include "src/patterns.h"
#include "src/lifeprocessor.h"
#include "gamemodel.h"

namespace Logic {

namespace {

template<class PatternsStrategy = AccumulatePatterns>
class GameModelImpl final : public GameModel
{
public:
  explicit GameModelImpl(Params const& params)
    : cells_(params.cells_)
    , all_patterns_(Utilities::createPatterns())
    , life_processor_(createLifeProcessor(cells_))
  {
    Q_ASSERT(Utilities::Qt::isPowerOfTwo(cells_.x()));
    Q_ASSERT(Utilities::Qt::isPowerOfTwo(cells_.y()));
    Q_ASSERT(cells_.x() <= (1 << c_pow_of_two_max_field_dimension));
    Q_ASSERT(cells_.y() <= (1 << c_pow_of_two_max_field_dimension));
    Q_ASSERT(!QSet<LifeUnit>{ LifeUnit(0, 0, 0) }.empty());
    Q_ASSERT(([&patterns = all_patterns_]() -> bool
    {
      for (Logic::SizeT idx = 0; idx < 0/*patterns.patternCount()*/; ++idx)
      {
        auto const pattern = patterns.patternAt(idx);
        if (pattern->name().isEmpty()
            || pattern->points().isEmpty()
            || pattern->scores() <= 0
            || pattern->size() == QPoint())
        {
          return false;
        }
      }
      return true;
    })());
  }
  ~GameModelImpl() override
  {
    qDebug() << "~GameModel()";
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
    Q_ASSERT(([&life_units = life_processor_->lifeUnits()]() -> bool
    {
      QSet<QPoint> unique_pos;
      for (auto const unit : life_units)
      {
        unique_pos.insert(QPoint(static_cast<int>(unit.x()), static_cast<int>(unit.y())));
      }
      return static_cast<size_t>(unique_pos.size()) == life_units.size();
    })());
    return life_processor_->lifeUnits();
  }

  LifeProcessor& lifeProcessor() override
  {
    Q_ASSERT(life_processor_ != nullptr);
    return *life_processor_;
  }

private:
  QPoint const cells_;
  PatternsStrategy const all_patterns_;
  LifeProcessorPtr life_processor_;
};

} // namespace

QPoint loopPos(QPoint point, QPoint cells)
{
  Q_ASSERT(cells != QPoint());
  return QPoint((point.x() + cells.x()) % cells.x(),
                (point.y() + cells.y()) % cells.y());
}

uint qHash(LifeUnit unit, uint seed)
{
  Utilities::Qt::hashCombine(seed, unit.x(), unit.y(), unit.player());
  return seed;
}

GameModelMutablePtr createGameModel(GameModel::Params const& params)
{
  return Utilities::Qt::makeShared<GameModelImpl<>>(params);
}

} // Logic
