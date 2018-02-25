#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <memory>
#include <QSharedPointer>
#include <QPoint>
#include <QSet>
#include <QVector>

namespace Logic {

using Points = QVector<QPoint>;
using SizeT = int;

struct Pattern
{
  virtual ~Pattern() = default;
  virtual QString name() const = 0;
  virtual Points const& points() const = 0;
  virtual QPoint size() const = 0;
  virtual SizeT scores() const = 0;
};
using PatternPtr = QSharedPointer<Pattern const>;

struct Patterns
{
  virtual ~Patterns() = default;
  virtual SizeT patternCount() const = 0;
  virtual PatternPtr patternAt(SizeT idx) const = 0;
};
using PatternsPtr = QSharedPointer<Patterns const>;

class GameField
{
public:
  struct Params
  {
    QPoint cells_;
  };

  explicit GameField(Params const& params);

  QPoint cells() const { return cells_; }
  PatternsPtr const& allPatterns() const { return all_patterns_; }

private:
  QPoint const cells_;
  PatternsPtr const all_patterns_;
};
using GameFieldPtr = std::unique_ptr<GameField>;

} // Logic

#endif // GAMELOGIC_H
