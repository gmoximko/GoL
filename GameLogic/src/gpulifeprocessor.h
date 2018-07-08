#ifndef GPULIFEPROCESSOR_H
#define GPULIFEPROCESSOR_H

#include "../gamemodel.h"

namespace Logic {

class GPULifeProcessor final : public LifeProcessor
{
public:
  explicit GPULifeProcessor(QPoint field_size);
  ~GPULifeProcessor() override;

  LifeUnits const& lifeUnits() const override
  {
    return life_units_;
  }
  bool computed() const override;
  int computationDuration() const override;

  void addUnit(LifeUnit unit) override;
  void processLife(bool compute) override;

private:
  void prepareLifeUnits();

  void* self_;
  QPoint field_size_;
  LifeUnits life_units_;
};

} // Logic

#endif // GPULIFEPROCESSOR_H
