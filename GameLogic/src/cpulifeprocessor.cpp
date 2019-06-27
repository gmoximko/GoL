#include "lifeprocessor.h"

namespace Logic {

namespace {

using Buffer = std::vector<uint8_t>;
using Point = QPoint;
using Index = uint32_t;

template<uint32_t Bit>
struct Mask
{
  constexpr static uint32_t c_n_e_s_w = 0x70007 << (Bit - 1);
  constexpr static uint32_t c_nw_ne_se_sw = 0x0;
  constexpr static uint32_t c_self = 0x5 << (Bit - 1);
};
template<>
struct Mask<0>
{
  constexpr static uint32_t c_n_e_s_w = 0x80030003;
  constexpr static uint32_t c_nw_ne_se_sw = 0x80000080;
  constexpr static uint32_t c_self = 0x2;
};
template<>
struct Mask<7>
{
  constexpr static uint32_t c_n_e_s_w = 0xC001C0;
  constexpr static uint32_t c_nw_ne_se_sw = 0x10100;
  constexpr static uint32_t c_self = 0x40;
};

template<uint32_t Bit>
uint32_t isAlive(uint32_t self, uint32_t n_e_s_w, uint32_t nw_ne_se_sw)
{
  /*
    nw0       n0        ne1
    [|||||||0][00333|77][7|||||||]
  w3[|||||||0][*03*3|7*][7|||||||]e1
    [|||||||0][00333|77][7|||||||]
    sw3       s2        se2 "
  */
  auto const popcount = [](uint32_t value)
  {
    uint32_t result = 0;
    while (value > 0)
    {
      result += value & 1;
      value >>= 1;
    }
    return result;
  };
  uint32_t neighbours = popcount(Mask<Bit>::c_n_e_s_w & n_e_s_w)
    + popcount(Mask<Bit>::c_nw_ne_se_sw & nw_ne_se_sw)
    + popcount(Mask<Bit>::c_self & self);

  return static_cast<uint32_t>((self >> Bit & 1) == 0
    ? neighbours == 3
    : neighbours == 2 || neighbours == 3) << Bit;
}

template<uint32_t Bit>
uint32_t calculateLife(uint32_t self, uint32_t n_e_s_w, uint32_t nw_ne_se_sw)
{
  return isAlive<Bit>(self, n_e_s_w, nw_ne_se_sw)
    | calculateLife<Bit - 1>(self, n_e_s_w, nw_ne_se_sw);
}
template<>
uint32_t calculateLife<0>(uint32_t self, uint32_t n_e_s_w, uint32_t nw_ne_se_sw)
{
  return isAlive<0>(self, n_e_s_w, nw_ne_se_sw);
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
    auto const gid = pos(id * 8);
    auto const nw = idx(loopPos(gid.x() - 8, gid.y() + 1));
    auto const n  = idx(loopPos(gid.x(),     gid.y() + 1));
    auto const ne = idx(loopPos(gid.x() + 8, gid.y() + 1));
    auto const e  = idx(loopPos(gid.x() + 8, gid.y()    ));
    auto const se = idx(loopPos(gid.x() + 8, gid.y() - 1));
    auto const s  = idx(loopPos(gid.x()    , gid.y() - 1));
    auto const sw = idx(loopPos(gid.x() - 8, gid.y() - 1));
    auto const w  = idx(loopPos(gid.x() - 8, gid.y()    ));

    auto const self = static_cast<uint32_t>(input[id]);
    auto const n_e_s_w = static_cast<uint32_t>(input[n >> 3]) << 0 * 8
      | static_cast<uint32_t>(input[e >> 3]) << 1 * 8
      | static_cast<uint32_t>(input[s >> 3]) << 2 * 8
      | static_cast<uint32_t>(input[w >> 3]) << 3 * 8;

    auto const nw_ne_se_sw = static_cast<uint32_t>(input[nw >> 3]) << 0 * 8
      | static_cast<uint32_t>(input[ne >> 3]) << 1 * 8
      | static_cast<uint32_t>(input[se >> 3]) << 2 * 8
      | static_cast<uint32_t>(input[sw >> 3]) << 3 * 8;

    output[id] = static_cast<uint8_t>(calculateLife<7>(self, n_e_s_w, nw_ne_se_sw));
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
    , input_(fieldSize() / 8)
    , output_(fieldSize() / 8)
    , computed_(1)
  {
    auto const thread_count = threadPool().maxThreadCount();
    auto const byte_count = fieldSize() / 8;
    auto const chunk_size = byte_count / thread_count;
    for (int idx = 0; idx < thread_count; ++idx)
    {
      auto const remainder = idx + 1 < thread_count ? 0 : byte_count % thread_count;
      auto const range = QPoint(chunk_size * idx, chunk_size * (idx + 1) + remainder);
      life_processes_.emplace_back(range, field_size, input_, output_, *this);
    }
  }
  ~CPULifeProcessor() override
  {
    while (!computed());
    qDebug() << "CPULifeProcessor min computaion duration " << min_computation_duration_;
  }

public: // LifeProcessor
  bool computed() const override;

protected: // LifeProcessorImpl
  void processLife() override;
  uint8_t* data() override
  {
    return input_.data();
  }

private:
  void handleComputeCompletion()
  {
    Q_ASSERT(!computed());
    Q_ASSERT(active_life_processes_ == 0);
    input_.swap(output_);
    min_computation_duration_ = std::min(min_computation_duration_, computation_duration_.elapsed());
    computed_.ref();
  }

  class LifeProcessChunk;
  std::vector<LifeProcessChunk> life_processes_;
  std::vector<uint8_t> input_;
  std::vector<uint8_t> output_;
  QTime computation_duration_;
  int min_computation_duration_ = std::numeric_limits<int>::max();
  QAtomicInt active_life_processes_;
  QAtomicInt computed_;
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

  void start()
  {
    ++processor_.active_life_processes_;
    processor_.threadPool().start(this);
  }
  void run() override
  {
    for (Index idx = range_.x(); idx < static_cast<Index>(range_.y()); ++idx)
    {
      life_process_.lifeStep(input_, output_, idx);
    }
    Q_ASSERT(processor_.active_life_processes_ > 0);
    if (--processor_.active_life_processes_ == 0)
    {
      processor_.handleComputeCompletion();
    }
  }

private:
  QPoint const range_;
  LifeProcess const life_process_;
  Buffer const& input_;
  Buffer& output_;
  CPULifeProcessor& processor_;
};

void CPULifeProcessor::processLife()
{
  Q_ASSERT(computed());
  computed_.deref();
  computation_duration_.start();
  for (auto& life_process : life_processes_)
  {
    life_process.start();
  }
}

bool CPULifeProcessor::computed() const
{
  return computed_ == 1;
}

} // namespace

LifeProcessorPtr createCPULifeProcessor(QPoint field_size)
{
  return std::make_unique<CPULifeProcessor>(field_size);
}

} // Logic
