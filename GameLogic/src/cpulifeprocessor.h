#ifndef CPULIFEPROCESSOR_H
#define CPULIFEPROCESSOR_H

#include <cstdint>

#include "../gamemodel.h"

namespace Logic {

class CPULifeProcessor : public LifeProcessor
{
public:
  explicit CPULifeProcessor(QPoint field_size);
  ~CPULifeProcessor() override;

  LifeUnits const& lifeUnits() const override
  {
    return life_units_;
  }

  void addUnit(LifeUnit const& unit) override;
  void processLife() override;

private:  
  bool computed() const;

  void prepareLifeUnits();
  void handleComputeCompletion();

  QPoint const field_size_;
  LifeUnits life_units_;

  class LifeProcessChunk;
  std::vector<LifeProcessChunk> life_processes_;
  std::vector<uint8_t> input_;
  std::vector<uint8_t> output_;

  QSet<decltype(input_.size())> position_cache_;
};

} // Logic

#endif // CPULIFEPROCESSOR_H
