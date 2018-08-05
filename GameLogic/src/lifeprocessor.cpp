#include <QDebug>

#include "lifeprocessor.h"

namespace Logic {

void LifeProcessorImpl::addUnit(LifeUnit unit)
{
  auto const position = unit.x() + unit.y() * field_size_.y();
  Q_ASSERT(computed());
  Q_ASSERT(position < fieldSize());
  data()[position] = unit.player() + 1;
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
  using VecType = uint64_t;

  life_units_.clear();

  Q_ASSERT(static_cast<VecType>(fieldSize()) % sizeof(VecType) == 0);
  auto const* begin = reinterpret_cast<VecType const*>(data());
  auto const* end   = reinterpret_cast<VecType const*>(data() + fieldSize());
  for (auto const* iter = begin; iter != end; ++iter)
  {
    auto const bytes = *iter;
    if (bytes == 0)
    {
      continue;
    }
    auto const index = static_cast<VecType>(iter - begin) * sizeof(VecType);
    for (VecType byte = 0; byte < sizeof(VecType); ++byte)
    {
      auto const life = bytes >> (byte * 8) & static_cast<VecType>(0xFF);
      if (life != 0)
      {
        auto const idx = index + byte;
        auto const x = static_cast<uint16_t>(idx % field_size_.x());
        auto const y = static_cast<uint16_t>(idx / field_size_.y());
        life_units_.emplace_back(LifeUnit(x, y));
      }
    }
  }
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
