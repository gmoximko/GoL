#include <QDebug>

#include "lifeprocessor.h"

namespace Logic {
/* unused
namespace {

template<typename Chunk>
void fillLifeUnits(uint8_t const* buffer, QPoint range, QPoint field_size, LifeUnits& life_units)
{
  auto const data = reinterpret_cast<Chunk const*>(buffer);
  auto const begin = data + range.x();
  auto const end   = data + range.y();

  for (auto const* iter = begin; iter != end; ++iter)
  {
    auto const bytes = *iter;
    if (bytes == 0)
    {
      continue;
    }
    auto const offset = static_cast<Chunk>(iter - begin + range.x()) * sizeof(Chunk);
    for (Chunk byte = 0; byte < sizeof(Chunk); ++byte)
    {
      auto const bits = bytes >> (byte * 8) & static_cast<Chunk>(0xFF);
      if (bits == 0)
      {
        continue;
      }
      for (Chunk bit = 0; bit < 8; ++bit)
      {
        auto const life = bits & (1 << bit);
        if (life != 0)
        {
          auto const idx = (offset + byte) * 8 + bit;
          auto const x = static_cast<uint16_t>(idx % field_size.x());
          auto const y = static_cast<uint16_t>(idx / field_size.y());
          life_units.emplace_back(LifeUnit(x, y));
        }
      }
    }
  }
}

} // namespace

template<typename Chunk>
class LifeProcessorImpl::PostProcess : public QRunnable
{
public:
  PostProcess(QPoint range, LifeProcessorImpl& processor)
    : range_(range)
    , processor_(processor)
  {
    setAutoDelete(false);
  }

  LifeUnits const& chunkUnits() const
  {
    return chunk_units_;
  }

  void start()
  {
    ++processor_.active_post_processes_;
    processor_.threadPool().start(this);
  }
  void run() override
  {
    chunk_units_.clear();
    fillLifeUnits<Chunk>(processor_.data(), range_, processor_.field_size_, chunk_units_);
    --processor_.active_post_processes_;
  }

private:
  QPoint const range_;
  LifeProcessorImpl& processor_;
  LifeUnits chunk_units_;
};
*/
LifeProcessorImpl::LifeProcessorImpl(QPoint field_size)
  : field_size_(field_size)
  , data_(fieldLength() / 8)
  , next_data_(fieldLength() / 8)
  , main_thread_(QThread::currentThread())
{
  Q_ASSERT(fieldLength() % 8 == 0);
}

LifeProcessorImpl::~LifeProcessorImpl()
{
  mainThread().check();
  qDebug() << "LifeProcessor min post process duration ";
}

LifeUnits const& LifeProcessorImpl::lifeUnits(QRect area) const
{
  life_units_.clear();
  for (uint16_t y = area.y(), y_end = y + area.height(); y < y_end; ++y)
  {
    for (uint16_t x = area.x(), x_end = x + area.width(); x < x_end; ++x)
    {
      LifeUnit const unit((x + cols()) % cols(), (y + rows()) % rows());
      if (getLife(unit, data_.data()))
      {
        life_units_.push_back(unit);
      }
    }
  }
  return life_units_;
}

void LifeProcessorImpl::init(QByteArray const& life_units)
{
  mainThread().check();
  onInit();
  loadLifeUnits(life_units);
  start();
}

void LifeProcessorImpl::destroy()
{
  mainThread().check();
  onDestroy();
  exit_.ref();
  quit();
  wait();
}

void LifeProcessorImpl::addUnits(LifeUnits units)
{
  mainThread().check();
  for (auto const unit : units)
  {
    setLife(unit, data_.data());
  }
//  life_units_.insert(life_units_.end(), units.cbegin(), units.cend());
  QMutexLocker locker(&mutex_);
  input_.insert(input_.end(), units.cbegin(), units.cend());
}

void LifeProcessorImpl::processLife(bool compute)
{
  mainThread().check();
  if (post_processed_)
  {
    if (compute)
    {
      data_.swap(next_data_);
      post_processed_.deref();
    }
  }
}

QByteArray LifeProcessorImpl::serialize() const
{
  mainThread().check();
  static_assert(std::is_same_v<uint8_t, char> || std::is_same_v<uint8_t, unsigned char>);
  return QByteArray(reinterpret_cast<char const*>(data_.data()), fieldLength() / 8);
}

void LifeProcessorImpl::run()
{
  computeThread().check();
/* unused
  Q_ASSERT(fieldLength() % 8 == 0);
  Q_ASSERT(fieldLength() / 8 % sizeof(Chunk) == 0);
  qDebug() << "Max threads: " << threadPool().maxThreadCount();

  std::vector<PostProcess<Chunk>> post_processes;
  auto const thread_count = threadPool().maxThreadCount();
  auto const chunks = fieldLength() / 8 / sizeof(Chunk);
  auto const chunk_size = chunks / thread_count;
  for (int idx = 0; idx < thread_count; ++idx)
  {
    auto const remainder = idx + 1 < thread_count ? 0 : chunk_size % thread_count;
    auto const range = QPoint(chunk_size * idx, chunk_size * (idx + 1) + remainder);
    post_processes.emplace_back(range, *this);
  }
*/
  for (; !exit_;)
  {
    if (!post_processed_ && computed())
    {
      updateData();
      std::memcpy(next_data_.data(), data(), next_data_.size());
      post_processed_.ref();
      processLife();
    }
  }
}
/* unused
void LifeProcessorImpl::prepareLifeUnits(std::vector<PostProcess<Chunk>> const& post_processes)
{
  computeThread().check();
  next_life_units_.clear();
  next_life_units_.resize(std::accumulate(post_processes.cbegin(), post_processes.cend(), SizeT(0),
    [](SizeT init, auto const& post_process)
    {
      return init + post_process.chunkUnits().size();
    }));

  SizeT units = 0;
  for (auto const& post_process : post_processes)
  {
    auto const& chunk_units = post_process.chunkUnits();
    std::memcpy(next_life_units_.data() + units,
                chunk_units.data(),
                chunk_units.size() * sizeof(LifeUnit));
    units += chunk_units.size();
  }
  Q_ASSERT(units == next_life_units_.size());
}

void LifeProcessorImpl::startAndWaitPostProcesses(std::vector<PostProcess<Chunk>>& post_processes)
{
  computeThread().check();
  Q_ASSERT(active_post_processes_ == 0);
  for (auto& post_process : post_processes)
  {
    post_process.start();
  }
  while (active_post_processes_ != 0);
}
*/

bool LifeProcessorImpl::getLife(LifeUnit unit, uint8_t const* data) const
{
  auto const position = unit.x() + unit.y() * cols();
  Q_ASSERT(position < fieldLength());
  auto const byte = position >> 3;
  auto const bit = position & 7;
  return (data[byte] & 1 << bit) != 0;
}

void LifeProcessorImpl::setLife(LifeUnit unit, uint8_t* data)
{
  auto const position = unit.x() + unit.y() * cols();
  Q_ASSERT(position < fieldLength());
  auto const byte = position >> 3;
  auto const bit = position & 7;
  data[byte] |= 1 << bit;
}

void LifeProcessorImpl::loadLifeUnits(QByteArray const& life_units)
{
  if (!life_units.isEmpty())
  {
    Q_ASSERT(fieldLength() == static_cast<SizeT>(life_units.size() * 8));
    std::memcpy(data(), life_units.data(), life_units.size());
    std::memcpy(data_.data(), life_units.data(), life_units.size());
//    fillLifeUnits<Chunk>(reinterpret_cast<uint8_t const*>(life_units.data()),
//                         QPoint(0, life_units.size() / sizeof(Chunk)),
//                         field_size_,
//                         life_units_);
  }
}

void LifeProcessorImpl::updateData()
{
  computeThread().check();
  Q_ASSERT(computed());
  QMutexLocker locker(&mutex_);
  for (auto const unit : input_)
  {
    setLife(unit, data());
  }
  input_.clear();
}

LifeProcessorPtr createLifeProcessor(QPoint field_size, QByteArray const& life_units)
{
  LifeProcessorPtr result;
  try
  {
    result = createGPULifeProcessor(field_size);
  }
  catch (std::exception const& e)
  {
    qDebug() << "Impossible to create GPULifeProcessor! " << e.what();
    result = createCPULifeProcessor(field_size);
  }
  Q_ASSERT(result != nullptr);
  result->init(life_units);
  return result;
}

} // Logic
