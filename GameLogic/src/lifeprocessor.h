#ifndef LIFEPROCESSOR_H
#define LIFEPROCESSOR_H

#include <QThreadPool>
#include <QDebug>
#include <QTime>

#include "../gamemodel.h"

namespace Logic {

class LifeProcessorImpl : public LifeProcessor
{
public:
  explicit LifeProcessorImpl(QPoint field_size);
  ~LifeProcessorImpl() override;

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
    return rows() * cols();
  }
  SizeT rows() const
  {
    return static_cast<SizeT>(field_size_.y());
  }
  SizeT cols() const
  {
    return static_cast<SizeT>(field_size_.x());
  }

protected:
  QThreadPool& threadPool() const
  {
    auto* result = QThreadPool::globalInstance();
    Q_ASSERT(result != nullptr);
    return *result;
  }

  virtual void processLife() = 0;
  virtual uint8_t* data() = 0;

private:
  void prepareLifeUnits();

  QPoint const field_size_;
  LifeUnits life_units_;

  template<typename Chunk>
  class PostProcess;
  using Chunk = uint64_t;
  std::vector<PostProcess<Chunk>> post_processes_;
  QAtomicInt active_post_processes_;
  QTime post_process_duration_;
  int min_post_process_duration_ = std::numeric_limits<int>::max();
};

LifeProcessorPtr createGPULifeProcessor(QPoint field_size);
LifeProcessorPtr createCPULifeProcessor(QPoint field_size);
LifeProcessorPtr createLifeProcessor(QPoint field_size);

} // Logic

#endif // LIFEPROCESSOR_H
