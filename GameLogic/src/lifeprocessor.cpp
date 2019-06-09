#include <QDebug>

#include "lifeprocessor.h"

namespace Logic {

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
  using Chunk = uint64_t;

  life_units_.clear();

  Q_ASSERT(static_cast<Chunk>(fieldSize()) % sizeof(Chunk) == 0);
  auto const* begin = reinterpret_cast<Chunk const*>(data());
  auto const* end   = reinterpret_cast<Chunk const*>(data() + (fieldSize() >> 3));
  for (auto const* iter = begin; iter != end; ++iter)
  {
    auto const bytes = *iter;
    if (bytes == 0)
    {
      continue;
    }
    auto const offset = static_cast<Chunk>(iter - begin) * sizeof(Chunk);
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
          auto const x = static_cast<uint16_t>(idx % cols());
          auto const y = static_cast<uint16_t>(idx / rows());
          life_units_.emplace_back(LifeUnit(x, y));
        }
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
