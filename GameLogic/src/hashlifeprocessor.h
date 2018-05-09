#ifndef HASHLIFEPROCESSOR_H
#define HASHLIFEPROCESSOR_H

#include "../gamemodel.h"

namespace Logic {

class HashLifeProcessor : public LifeProcessor
{
public:
  explicit HashLifeProcessor(QPoint field_size)
    : field_size_(field_size)
  {}

  LifeUnits const& lifeUnits() const override
  {
    return life_units_;
  }

  void addUnit(LifeUnit const& life_unit) override
  {
    life_units_.insert(life_unit);
  }
  void processLife() override
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
            auto const neighbour_unit = loopPos(unit + neighbour, field_size_);
            new_life_units[neighbour_unit]++;
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
  QPoint const field_size_;
  LifeUnits life_units_;
};

} // Logic

#endif // HASHLIFEPROCESSOR_H
