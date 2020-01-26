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
    processor_.threadPool().start(this);
  }
  void run() override
  {
    auto const data = reinterpret_cast<Chunk const*>(processor_.data());
    auto const begin = data + range_.x();
    auto const end   = data + range_.y();

    chunk_units_.clear();
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
  , main_thread_(QThread::currentThread())
{}

LifeProcessorImpl::~LifeProcessorImpl()
{
  mainThread().check();
  qDebug() << "LifeProcessor min post process duration ";
}

void LifeProcessorImpl::init()
{
  mainThread().check();
  onInit();
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
  life_units_.insert(life_units_.end(), units.cbegin(), units.cend());
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
      life_units_.swap(next_life_units_);
      post_processed_.deref();
    }
  }
}

void LifeProcessorImpl::run()
{
  computeThread().check();

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

  for (; !exit_;)
  {
    if (!post_processed_ && computed())
    {
      startAndWaitPostProcesses(post_processes);
      prepareLifeUnits(post_processes);
      post_processed_.ref();
      updateData();
      processLife();
    }
  }
}

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

void LifeProcessorImpl::updateData()
{
  computeThread().check();
  Q_ASSERT(computed());
  QMutexLocker locker(&mutex_);
  for (auto const unit : input_)
  {
    auto const position = unit.x() + unit.y() * rows();
    Q_ASSERT(position < fieldLength());
    auto const byte = position >> 3;
    auto const bit = position & 7;
    data()[byte] |= 1 << bit;
  }
  input_.clear();
}

LifeProcessorPtr createLifeProcessor(QPoint field_size)
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
  result->init();
  return result;
}

} // Logic
