#ifndef GPULIFEPROCESSOR_H
#define GPULIFEPROCESSOR_H

#include "../gamemodel.h"

namespace Logic {

class GPULifeProcessor : public LifeProcessor
{
public:
  explicit GPULifeProcessor(QPoint field_size);
  ~GPULifeProcessor() override;

  LifeUnits const& lifeUnits() const override
  {
    return life_units_;
  }

  void addUnit(LifeUnit unit) override;
  void processLife() override;

private:
  void* self_;
  QPoint field_size_;
  LifeUnits life_units_;
};

} // Logic

#endif // GPULIFEPROCESSOR_H
