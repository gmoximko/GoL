#ifndef GPULIFEPROCESSOR_H
#define GPULIFEPROCESSOR_H

#include <QPoint>

#include "../gamemodel.h"

namespace Logic {

class GPULifeProcessor
{
public:
  explicit GPULifeProcessor(QPoint field_size);
  ~GPULifeProcessor();

  LifeUnits const& lifeUnits() const
  {
    return life_units_;
  }

  void processLife();
  void addUnit(LifeUnit const& unit);

private:
  void* self_;
  QPoint field_size_;
  LifeUnits life_units_;
};

} // Logic

#endif // GPULIFEPROCESSOR_H
