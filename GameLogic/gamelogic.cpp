#include "gamelogic.h"
#include "../Utilities/rleparser.h"

namespace Logic {

GameField::GameField(Params const& params)
  : cells_(params.cells_)
  , all_patterns_(Utilities::preparePatterns())
{
#if defined(QT_DEBUG)
  for (Logic::SizeT idx = 0; idx < all_patterns_->patternCount(); ++idx)
  {
    auto const pattern = all_patterns_->patternAt(idx);
    Q_ASSERT(!pattern->name().isEmpty());
    Q_ASSERT(!pattern->points().isEmpty());
    Q_ASSERT(pattern->scores() > 0);
    Q_ASSERT(pattern->size() != QPoint());
  }
#endif
}

} // Logic
