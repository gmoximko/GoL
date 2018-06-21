#include <QHash>
#include <QDebug>

#include "../Utilities/qtutilities.h"
#include "../Utilities/rleparser.h"
#include "src/patterns.h"
#include "src/gpulifeprocessor.h"
#include "src/cpulifeprocessor.h"
#include "gamemodel.h"

namespace Logic {

namespace {

LifeProcessorPtr makeLifeProcessor(QPoint cells)
{
  try
  {
    return std::make_unique<GPULifeProcessor>(cells);
  }
  catch(std::exception const& e)
  {
    qDebug() << "Impossible to create GPULifeProcessor! " << e.what();
    return std::make_unique<CPULifeProcessor>(cells);
  }
}

template<class PatternsStrategy = AccumulatePatterns>
class GameModelImpl : public GameModel
{
public:
  explicit GameModelImpl(Params const& params)
    : cells_(params.cells_)
    , all_patterns_(Utilities::createPatterns())
    , life_processor_(makeLifeProcessor(cells_))
  {
    Q_ASSERT(!QSet<LifeUnit>{ LifeUnit(0, 0, 0) }.empty());
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

  void addUnit(QPoint position, PlayerId player) override
  {
    position = loopPos(position, cells_);
    life_processor_->addUnit(LifeUnit(static_cast<uint16_t>(position.x()),
                                      static_cast<uint16_t>(position.y()), player));
  }
  void makeStep() override
  {
    life_processor_->processLife();
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
