#include <QHash>
#include <QDebug>

#include "gamemodel.h"
#include "../Utilities/qtutilities.h"
#include "../Utilities/rleparser.h"

uint qHash(QPoint const& unit)
{
  return qHash(qMakePair(unit.x(), unit.y()));
}

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
    life_units_.insert(life_unit);
  }
  void makeStep() override
  {
    QHash<QPoint, int> new_life_units;
    for (auto const& unit : life_units_)
    {
      for (int x = -1; x <= 1; ++x)
      {
        for (int y = -1; y <= 1; ++y)
        {
          QPoint neighbour(x, y);
          if (neighbour != QPoint())
          {
            new_life_units[unit + neighbour]++;
          }
          else if (!new_life_units.contains(neighbour))
          {
            new_life_units[unit];
          }
        }
      }
    }
    for (auto it = new_life_units.begin(); it != new_life_units.end(); ++it)
    {
      if (it.value() == 3)
      {
        life_units_.insert(it.key());
      }
      else if (it.value() < 2 || it.value() > 3)
      {
        auto to_erase = life_units_.find(it.key());
        if (to_erase != life_units_.end())
        {
          life_units_.erase(to_erase);
        }
      }
    }
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
