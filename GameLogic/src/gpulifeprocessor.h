#ifndef GPULIFEPROCESSOR_H
#define GPULIFEPROCESSOR_H

#include "../gamemodel.h"

namespace Logic {

class GPULifeProcessor
{
public:
  void addUnit(LifeUnit const& life_unit);
  void processLife();

private:
  class Impl;
  QScopedPointer<Impl> pimpl_;
};

} // Logic

#endif // GPULIFEPROCESSOR_H
