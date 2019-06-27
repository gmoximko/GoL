#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <memory>

#include <QSharedPointer>
#include <QPoint>
#include <QVector>
#include <QMatrix>
#include <QSet>

namespace Logic {

using Points = QVector<QPoint>;
using SizeT = uint32_t;
using PlayerId = uint8_t;
using Score = uint64_t;

constexpr uint32_t c_pow_of_two_max_field_dimension = 16;
constexpr PlayerId c_max_player_count = 4;

QPoint loopPos(QPoint point, QPoint cells);

struct Pattern
{
  virtual ~Pattern() = default;
  virtual QString name() const = 0;
  virtual Points const& points() const = 0;
  virtual QPoint size() const = 0;
  virtual Score scores() const = 0;
};
using PatternPtr = QSharedPointer<Pattern const>;
using PatternTrs = QPair<PatternPtr, QMatrix>;

class LifeUnit
{
  constexpr static uint32_t c_coordinate_mask = (1 << c_pow_of_two_max_field_dimension) - 1;

public:
  LifeUnit() = default;
  LifeUnit(uint16_t x, uint16_t y)
    : x_(x)
    , y_(y)
  {
//    Q_ASSERT(x < (1 << c_pow_of_two_max_field_dimension));
//    Q_ASSERT(y < (1 << c_pow_of_two_max_field_dimension));
  }
  uint16_t x() const
  {
    return x_;
  }
  uint16_t y() const
  {
    return y_;
  }
  bool operator == (LifeUnit rhs) const
  {
    return x_ == rhs.x_ && y_ == rhs.y_;
  }
  bool operator != (LifeUnit rhs) const
  {
    return !(*this == rhs);
  }

private:
  uint16_t x_ = 0;
  uint16_t y_ = 0;
};
static_assert(sizeof(LifeUnit) == sizeof(uint32_t), "sizeof(LifeUnit)");
static_assert(std::is_trivially_copyable_v<LifeUnit>, "");
uint qHash(LifeUnit unit, uint seed);
using LifeUnits = std::vector<LifeUnit>;

struct LifeProcessor
{
  virtual ~LifeProcessor() = default;
  virtual LifeUnits const& lifeUnits() const = 0;
  virtual bool computed() const = 0;

  virtual void addUnit(LifeUnit unit) = 0;
  virtual void processLife(bool compute) = 0;
};
using LifeProcessorPtr = std::unique_ptr<LifeProcessor>;
LifeProcessorPtr createLifeProcessor(QPoint field_size);

struct GameModel
{
  struct Params
  {
    QPoint cells_;
  };

  virtual ~GameModel() = default;
  virtual QPoint cells() const = 0;
  virtual SizeT patternCount() const = 0;
  virtual PatternPtr patternAt(SizeT idx) const = 0;
  virtual LifeUnits const& lifeUnits() const = 0;

  virtual LifeProcessor& lifeProcessor() = 0;
};
using GameModelPtr = QSharedPointer<GameModel const>;
using GameModelMutablePtr = QSharedPointer<GameModel>;
GameModelMutablePtr createGameModel(GameModel::Params const& params);

} // Logic

#endif // GAMEMODEL_H
