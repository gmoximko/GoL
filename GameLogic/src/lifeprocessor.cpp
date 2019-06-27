#include <QDebug>

#include "lifeprocessor.h"

namespace Logic {

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
    chunk_units_.clear();
    processor_.threadPool().start(this);
  }
  void run() override
  {
    auto const data = reinterpret_cast<Chunk const*>(processor_.data());
    auto const begin = data + range_.x();
    auto const end   = data + range_.y();

    for (auto const* iter = begin; iter != end; ++iter)
    {
      auto const bytes = *iter;
      if (bytes == 0)
      {
        continue;
      }
      auto const offset = static_cast<Chunk>(iter - begin + range_.x()) * sizeof(Chunk);
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
            auto const x = static_cast<uint16_t>(idx % processor_.cols());
            auto const y = static_cast<uint16_t>(idx / processor_.rows());
            chunk_units_.emplace_back(LifeUnit(x, y));
          }
        }
      }
    }
    --processor_.active_post_processes_;
  }

private:
  QPoint const range_;
  LifeProcessorImpl& processor_;
  LifeUnits chunk_units_;
};

LifeProcessorImpl::LifeProcessorImpl(QPoint field_size)
  : field_size_(field_size)
{
  Q_ASSERT(fieldSize() % 8 == 0);
  Q_ASSERT(fieldSize() / 8 % sizeof(Chunk) == 0);

  qDebug() << "Max threads: " << threadPool().maxThreadCount();
  auto const thread_count = threadPool().maxThreadCount();
  auto const chunks = fieldSize() / 8 / sizeof(Chunk);
  auto const chunk_size = chunks / thread_count;
  for (int idx = 0; idx < thread_count; ++idx)
  {
    auto const remainder = idx + 1 < thread_count ? 0 : chunk_size % thread_count;
    auto const range = QPoint(chunk_size * idx, chunk_size * (idx + 1) + remainder);
    post_processes_.emplace_back(range, *this);
  }
}

LifeProcessorImpl::~LifeProcessorImpl()
{
  qDebug() << "LifeProcessor min post process duration " << min_post_process_duration_;
}

void LifeProcessorImpl::addUnit(LifeUnit unit)
{
  auto const position = unit.x() + unit.y() * rows();
  Q_ASSERT(computed());
  Q_ASSERT(position < fieldSize());
  auto const byte = position >> 3;
  auto const bit = position & 7;
  data()[byte] |= 1 << bit;
}

void LifeProcessorImpl::processLife(bool compute)
{
  if (!computed())
  {
    return;
  }
  prepareLifeUnits();
  if (compute)
  {
    processLife();
  }
}

void LifeProcessorImpl::prepareLifeUnits()
{
  Q_ASSERT(active_post_processes_ == 0);
  post_process_duration_.start();
  life_units_.clear();
  for (auto& post_process : post_processes_)
  {
    post_process.start();
  }
  while (active_post_processes_ != 0);

  life_units_.resize(std::accumulate(post_processes_.cbegin(), post_processes_.cend(), SizeT(0),
    [](SizeT init, auto const& post_process)
    {
      return init + post_process.chunkUnits().size();
    }));

  SizeT units = 0;
  for (auto const& post_process : post_processes_)
  {
    auto const& chunk_units = post_process.chunkUnits();
    std::memcpy(life_units_.data() + units, chunk_units.data(), chunk_units.size() * sizeof(LifeUnit));
    units += chunk_units.size();
  }
  min_post_process_duration_ = std::min(min_post_process_duration_, post_process_duration_.elapsed());
  Q_ASSERT(units == life_units_.size());
}

LifeProcessorPtr createLifeProcessor(QPoint field_size)
{
  try
  {
    return createGPULifeProcessor(field_size);
  }
  catch(std::exception const& e)
  {
    qDebug() << "Impossible to create GPULifeProcessor! " << e.what();
    return createCPULifeProcessor(field_size);
  }
}

} // Logic
