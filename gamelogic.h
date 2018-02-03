#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <memory>
#include <QPoint>
#include <QSet>
#include <QVector>

namespace Logic {

using Unit = QPoint;
using Units = QSet<Unit>;

struct GameField
{
  struct Params
  {
    QPoint cells_;
  };

  virtual ~GameField() = default;
  virtual QPoint cells() const = 0;
//  virtual QVector<Units> const& GetAllPatterns() const = 0;
//  virtual Units const& GetAllUnits() const = 0;
};
using GameFieldPtr = std::unique_ptr<GameField>;

GameFieldPtr createGameField(GameField::Params const& params);

} // Logic

#endif // GAMELOGIC_H
