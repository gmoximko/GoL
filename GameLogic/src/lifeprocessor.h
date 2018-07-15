#ifndef LIFEPROCESSOR_H
#define LIFEPROCESSOR_H

#include "../gamemodel.h"

namespace Logic {

class LifeProcessorImpl : public LifeProcessor
{
public:
  explicit LifeProcessorImpl(QPoint field_size)
    : field_size_(field_size)
  {}

public: // LifeProcessor
  LifeUnits const& lifeUnits() const final
  {
    return life_units_;
  }

  void addUnit(LifeUnit unit) final;
  void processLife(bool compute) final;

public:
  SizeT fieldSize() const
  {
    return field_size_.x() * field_size_.y();
  }

protected:
  virtual void processLife() = 0;
  virtual uint8_t* data() = 0;

private:
  void prepareLifeUnits();

  QPoint const field_size_;
  LifeUnits life_units_;
};

LifeProcessorPtr createGPULifeProcessor(QPoint field_size);
LifeProcessorPtr createCPULifeProcessor(QPoint field_size);
LifeProcessorPtr createLifeProcessor(QPoint field_size);

} // Logic

#endif // LIFEPROCESSOR_H
