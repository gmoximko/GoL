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
    return life_processor_->lifeUnits();
  }

  void addUnit(QPoint position, uint32_t player) override
  {
    position = loopPos(position, cells_);
    life_processor_->addUnit(LifeUnit(position.x(), position.y(), player));
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

GameModelMutablePtr createGameModel(GameModel::Params const& params)
{
  return Utilities::Qt::makeShared<GameModelImpl<>>(params);
}

} // Logic
