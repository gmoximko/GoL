#include <QThreadPool>
#include <QDebug>

#include "cpulifeprocessor.h"

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

} // namespace

class CPULifeProcessor::LifeProcessChunk : public QRunnable
{
public:
  explicit LifeProcessChunk(QPoint range, QPoint field_size, Buffer const& input, Buffer& output)
    : range_(range)
    , life_process_(field_size.x(), field_size.y())
    , input_(input)
    , output_(output)
  {
    setAutoDelete(false);
  }

  bool computed() const
  {
    return computed_;
  }

  void start()
  {
    computed_ = false;
    threadPool().start(this);
  }
  void run() override
  {
    for (Index idx = range_.x(); idx < static_cast<Index>(range_.y()); ++idx)
    {
      life_process_.lifeStep(input_, output_, idx);
    }
    computed_ = true;
  }

private:
  QPoint const range_;
  LifeProcess const life_process_;
  Buffer const& input_;
  Buffer& output_;
  bool computed_ = true;
};

CPULifeProcessor::CPULifeProcessor(QPoint field_size)
  : field_size_(field_size)
  , input_(field_size_.x() * field_size_.y())
  , output_(field_size_.x() * field_size_.y())
{
  qDebug() << "Active threads: " << threadPool().activeThreadCount()
           << " Max threads: " << threadPool().maxThreadCount();

  auto const thread_count = threadPool().maxThreadCount();
  auto const chunk_size = (field_size_.x() * field_size_.y()) / thread_count;
  for (int idx = 0; idx < thread_count; ++idx)
  {
    auto const range = QPoint(chunk_size * idx, chunk_size * (idx + 1));
    life_processes_.push_back(LifeProcessChunk(range, field_size_, input_, output_));
  }
}

CPULifeProcessor::~CPULifeProcessor()
{
  while (!computed());
}

void CPULifeProcessor::addUnit(LifeUnit const& unit)
{
  auto const position = static_cast<Index>(unit.x() + unit.y() * field_size_.y());
  if (computed())
  {
    Q_ASSERT(position < input_.size());
    input_[position] = 1;
  }
  else
  {
    position_cache_.insert(position);
  }
}

void CPULifeProcessor::processLife()
{
  prepareLifeUnits();
  if (!computed())
  {
    return;
  }
  handleComputeCompletion();
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

void CPULifeProcessor::prepareLifeUnits()
{
  life_units_.clear();

  for (Index idx = 0; idx < input_.size(); ++idx)
  {
    if (input_[idx] != 0)
    {
      auto const x = static_cast<int>(idx % field_size_.x());
      auto const y = static_cast<int>(idx / field_size_.y());
      life_units_.insert(LifeUnit(x, y));
    }
  }
}

void CPULifeProcessor::handleComputeCompletion()
{
  input_.swap(output_);
  for (auto const& pos : position_cache_)
  {
    input_[pos] = 1;
  }
  position_cache_.clear();
}

} // Logic
