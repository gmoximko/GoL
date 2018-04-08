#ifndef GAMEMODELPATTERNS_H
#define GAMEMODELPATTERNS_H

#include <QVector>

#include "../../Utilities/rleparser.h"

namespace Logic {

class SimplePatterns
{
public:
  explicit SimplePatterns(Utilities::PatternsPtr const all_patterns)
    : all_patterns_(all_patterns)
  {}

  Logic::SizeT patternCount() const
  {
    return all_patterns_->patternCount();
  }
  PatternPtr patternAt(Logic::SizeT idx) const
  {
    return all_patterns_->parsePatternAt(idx);
  }

private:
  Utilities::PatternsPtr const all_patterns_;
};

class AccumulatePatterns
{
public:
  explicit AccumulatePatterns(Utilities::PatternsPtr const all_patterns)
    : all_patterns_(all_patterns)
    , accumulated_patterns_(all_patterns_->patternCount())
  {}

  Logic::SizeT patternCount() const
  {
    return all_patterns_->patternCount();
  }
  PatternPtr patternAt(Logic::SizeT idx) const
  {
    auto pattern_ptr = accumulated_patterns_.at(idx);
    if (pattern_ptr == nullptr)
    {
      pattern_ptr = all_patterns_->parsePatternAt(idx);
      accumulated_patterns_[idx] = pattern_ptr;
    }
    return pattern_ptr;
  }

private:
  Utilities::PatternsPtr const all_patterns_;
  QVector<PatternPtr> mutable accumulated_patterns_;
};

} // Logic

#endif // GAMEMODELPATTERNS_H
