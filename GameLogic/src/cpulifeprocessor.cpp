#include <QThreadPool>
#include <QDebug>
#include <QTime>

#include "lifeprocessor.h"

namespace Logic {

namespace {

using Buffer = std::vector<uint8_t>;
using Point = QPoint;
using Index = size_t;

QThreadPool& threadPool()
{
  auto* result = QThreadPool::globalInstance();
  Q_ASSERT(result != nullptr);
  return *result;
}

class LifeProcess
{
public:
  explicit LifeProcess(Index width, Index height)
    : width_(width)
    , height_(height)
  {}

  void lifeStep(Buffer const& input, Buffer& output, Index id) const
  {
    auto const gid = pos(id);
    auto neighbours = 0;
    for (int x = -1; x <= 1; ++x)
    {
      for (int y = -1; y <= 1; ++y)
      {
        auto const index = idx(loopPos(gid.x() + x, gid.y() + y));
        Q_ASSERT(index < width_ * height_);
        if (index != id && input[index] != 0)
        {
          ++neighbours;
        }
      }
    }
    auto const self = input[id];
    output[id] = (self == 0) ? (neighbours == 3) : (neighbours == 2 || neighbours == 3);
  }

private:
  Point pos(Index id) const
  {
    return Point(static_cast<int>(id % width_),
                 static_cast<int>(id / height_));
  }
  Index idx(Point pos) const
  {
    return pos.x() + pos.y() * height_;
  }
  Point loopPos(int16_t x, int16_t y) const
  {
    return Point(static_cast<int>((x + width_) % width_),
                 static_cast<int>((y + height_) % height_));
  }

  Index const width_ = 0;
  Index const height_ = 0;
};

class CPULifeProcessor final : public LifeProcessorImpl
{
public:
  explicit CPULifeProcessor(QPoint field_size)
    : LifeProcessorImpl(field_size)
    , input_(field_size.x() * field_size.y())
    , output_(field_size.x() * field_size.y())
  {
    qDebug() << "Active threads: " << threadPool().activeThreadCount()
             << " Max threads: " << threadPool().maxThreadCount();
    auto const thread_count = threadPool().maxThreadCount();
    auto const chunk_size = (field_size.x() * field_size.y()) / thread_count;
    for (int idx = 0; idx < thread_count; ++idx)
    {
      auto const range = QPoint(chunk_size * idx, chunk_size * (idx + 1));
      life_processes_.emplace_back(range, field_size, input_, output_, *this);
    }
  }
  ~CPULifeProcessor() override
  {
    while (!computed());
    QMutexLocker locker(&mutex_);
  }

public: // LifeProcessor
  bool computed() const override;
  int computationDuration() const override
  {
    return last_computation_duration_;
  }

protected: // LifeProcessorImpl
  void processLife() override;
  uint8_t* data() override
  {
    return input_.data();
  }

private:
  void handleComputeCompletion()
  {
    if (computed())
    {
      input_.swap(output_);
      last_computation_duration_ = computation_duration_.elapsed();
    }
  }

  class LifeProcessChunk;
  std::vector<LifeProcessChunk> life_processes_;
  std::vector<uint8_t> input_;
  std::vector<uint8_t> output_;
  QMutex mutex_;
  QTime computation_duration_;
  int last_computation_duration_ = 0;
};

class CPULifeProcessor::LifeProcessChunk final : public QRunnable
{
public:
  explicit LifeProcessChunk(QPoint range, QPoint field_size, Buffer const& input, Buffer& output, CPULifeProcessor& processor)
    : range_(range)
    , life_process_(field_size.x(), field_size.y())
    , input_(input)
    , output_(output)
    , processor_(processor)
  {
    setAutoDelete(false);
  }

  bool computed() const
  {
    return computed_;
  }

  void start()
  {
    Q_ASSERT(computed());
    computed_ = false;
    threadPool().start(this);
  }
  void run() override
  {
    for (Index idx = range_.x(); idx < static_cast<Index>(range_.y()); ++idx)
    {
      life_process_.lifeStep(input_, output_, idx);
    }
    QMutexLocker locker(&processor_.mutex_);
    computed_ = true;
    processor_.handleComputeCompletion();
  }

private:
  QPoint const range_;
  LifeProcess const life_process_;
  Buffer const& input_;
  Buffer& output_;
  CPULifeProcessor& processor_;
  bool computed_ = true;
};

void CPULifeProcessor::processLife()
{
  computation_duration_.start();
  for (auto& life_process : life_processes_)
  {
    life_process.start();
  }
}

bool CPULifeProcessor::computed() const
{
  return std::all_of(life_processes_.begin(), life_processes_.end(), [](LifeProcessChunk const& chunk)
  {
    return chunk.computed();
  });
}

} // namespace

LifeProcessorPtr createCPULifeProcessor(QPoint field_size)
{
  return std::make_unique<CPULifeProcessor>(field_size);
}

} // Logic
