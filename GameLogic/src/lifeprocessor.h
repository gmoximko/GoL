#ifndef LIFEPROCESSOR_H
#define LIFEPROCESSOR_H

#include <QThreadPool>
#include <QDebug>
#include <QTime>

#include "../../Utilities/qtutilities.h"
#include "../gamemodel.h"

namespace Logic {

class LifeProcessorImpl : public LifeProcessor, QThread
{
public:
  explicit LifeProcessorImpl(QPoint field_size);
  ~LifeProcessorImpl() override;

public: // LifeProcessor
  LifeUnits const& lifeUnits() const final
  {
    return life_units_;
  }

  void init() final;
  void destroy() final;
  void addUnits(LifeUnits units) final;
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
  Utilities::Qt::ThreadChecker const& mainThread() const { return main_thread_; }
  Utilities::Qt::ThreadChecker const& computeThread() const { return compute_thread_; }

  virtual void onInit() {}
  virtual void onDestroy() {}
  virtual void processLife() = 0;
  virtual uint8_t* data() = 0;

private: // QThread
  void run() final;

private:
  template<typename Chunk>
  class PostProcess;
  using Chunk = uint64_t;

  void prepareLifeUnits(std::vector<PostProcess<Chunk>> const& post_processes);
  void startAndWaitPostProcesses(std::vector<PostProcess<Chunk>>& post_processes);
  void updateData();

  QPoint const field_size_;
  LifeUnits life_units_;
  LifeUnits next_life_units_;
  QAtomicInt active_post_processes_;
  QAtomicInt post_processed_;
  QAtomicInt exit_;
  QMutex mutex_;
  LifeUnits input_;
  Utilities::Qt::ThreadChecker main_thread_;
  Utilities::Qt::ThreadChecker compute_thread_{this};
};

LifeProcessorPtr createGPULifeProcessor(QPoint field_size);
LifeProcessorPtr createCPULifeProcessor(QPoint field_size);
LifeProcessorPtr createLifeProcessor(QPoint field_size);

} // Logic

#endif // LIFEPROCESSOR_H
