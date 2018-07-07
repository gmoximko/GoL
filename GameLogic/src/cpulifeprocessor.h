#ifndef CPULIFEPROCESSOR_H
#define CPULIFEPROCESSOR_H

#include <cstdint>

#include "../gamemodel.h"

namespace Logic {

class CPULifeProcessor final : public LifeProcessor
{
public:
  explicit CPULifeProcessor(QPoint field_size);
  ~CPULifeProcessor() override;

  LifeUnits const& lifeUnits() const override
  {
    return life_units_;
  }
  bool computed() const override;

  void addUnit(LifeUnit unit) override;
  void processLife() override;

private:  
  void prepareLifeUnits();
  void handleComputeCompletion();

  QPoint const field_size_;
  LifeUnits life_units_;

  class LifeProcessChunk;
  std::vector<LifeProcessChunk> life_processes_;
  std::vector<uint8_t> input_;
  std::vector<uint8_t> output_;
};

} // Logic

#endif // CPULIFEPROCESSOR_H
