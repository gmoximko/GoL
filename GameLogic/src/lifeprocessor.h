#ifndef LIFEPROCESSOR_H
#define LIFEPROCESSOR_H

#include <QSet>
#include <QHash>

#include "gpulifeprocessor.h"

uint qHash(QPoint const& unit)
{
  return qHash(qMakePair(unit.x(), unit.y()));
}

namespace Logic {

class SimpleLifeProcessor
{
public:
  LifeUnits const& lifeUnits() const
  {
    return life_units_;
  }

  void addUnit(LifeUnit const& life_unit)
  {
    life_units_.insert(life_unit);
  }
  void processLife()
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
  LifeUnits life_units_;
};

} // Logic

#endif // LIFEPROCESSOR_H
