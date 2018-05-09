#ifndef CPULIFEPROCESSOR_H
#define CPULIFEPROCESSOR_H

#include <vector>
#include <set>
#include <cstdint>
#include <future>

#include "../gamemodel.h"

namespace Logic {

class CPULifeProcessor : public LifeProcessor
{
public:
  explicit CPULifeProcessor(QPoint field_size);

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

  std::vector<uint8_t> input_;
  std::vector<uint8_t> output_;

  std::vector<std::future<void>> futures_;
  std::set<size_t> position_cache_;
};

} // Logic

#endif // CPULIFEPROCESSOR_H
