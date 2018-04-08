#ifndef RLEPARSER_H
#define RLEPARSER_H

#include <QVector>
#include <QSet>
#include <QPoint>

#include "../GameLogic/gamemodel.h"

namespace Utilities {

struct Patterns
{
  virtual ~Patterns() = default;
  virtual Logic::SizeT patternCount() const = 0;
  virtual Logic::PatternPtr parsePatternAt(Logic::SizeT idx) const = 0;
};
using PatternsPtr = QSharedPointer<Patterns const>;
PatternsPtr createPatterns();

} // Utilities

#endif // RLEPARSER_H
