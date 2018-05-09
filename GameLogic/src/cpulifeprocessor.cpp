#include "cpulifeprocessor.h"

namespace Logic {

namespace {

class LifeProcess
{
public:
  using Buffer = std::vector<uint8_t>;
  using Point = QPoint;
  using Index = uint64_t;

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
        auto const index = idx(gid + Point(x, y));
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
    return Point(id % width_, id / height_);
  }
  Index idx(Point pos) const
  {
    return pos.x() + pos.y() * height_;
  }
  Point loopPos(int16_t x, int16_t y) const
  {
    return Point((x + width_) % width_, (y + height_) % height_);
  }

  Index const width_ = 0;
  Index const height_ = 0;
};

} // namespace

CPULifeProcessor::CPULifeProcessor(QPoint field_size)
  : field_size_(field_size)
  , input_(field_size_.x() * field_size_.y())
  , output_(field_size_.x() * field_size_.y())
{}

void CPULifeProcessor::addUnit(LifeUnit const& unit)
{
  auto const position = static_cast<size_t>(unit.x() + unit.y() * field_size_.y());
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

  LifeProcess process(field_size_.x(), field_size_.y());
  auto process_chunk = [process, this](size_t begin, size_t end)
  {
    for (size_t idx = begin; idx < end; ++idx)
    {
      process.lifeStep(input_, output_, idx);
    }
  };

  auto const hardware_concurrency = std::thread::hardware_concurrency();
  auto const chunk_size = input_.size() / hardware_concurrency;
  for (size_t idx = 0; idx < hardware_concurrency; ++idx)
  {
    futures_.emplace_back(
          std::async(std::launch::async, process_chunk, chunk_size * idx, chunk_size * (idx + 1)));
  }
}

bool CPULifeProcessor::computed() const
{
  return std::all_of(futures_.begin(), futures_.end(), [](std::future<void> const& future)
  {
    return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
  });
}

void CPULifeProcessor::prepareLifeUnits()
{
  life_units_.clear();

  for (size_t idx = 0; idx < input_.size(); ++idx)
  {
    if (input_[idx] != 0)
    {
      int x = idx % field_size_.x();
      int y = idx / field_size_.y();
      life_units_.insert(LifeUnit(x, y));
    }
  }
}

void CPULifeProcessor::handleComputeCompletion()
{
  futures_.clear();
  input_.swap(output_);
  for (auto const& pos : position_cache_)
  {
    input_[pos] = 1;
  }
  position_cache_.clear();
}

} // Logic
