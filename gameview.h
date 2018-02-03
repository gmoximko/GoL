#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <QQuickPaintedItem>
#include <QPoint>

#include "gamelogic.h"

namespace View {

class GameView : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QPoint fieldSize READ fieldSize CONSTANT)

public:
  using QQuickPaintedItem::QQuickPaintedItem;

  QPoint fieldSize() const
  {
    auto const cells = gameField().cells();
    return QPoint(cells.x() * pixels_per_cell_.x(),
                  cells.y() * pixels_per_cell_.y());
  }

  void setGameField(Logic::GameField* game_field) { game_field_ = game_field; }
  void paint(QPainter* painter) override;
  Q_INVOKABLE void pressed(QPointF point);

private:
  Logic::GameField& gameField() const
  {
    Q_ASSERT(game_field_ != nullptr);
    return *game_field_;
  }

  void drawGrid(QPainter* painter) const;
  void drawCoordinates(QPainter* painter) const;

  QPoint const pixels_per_cell_ = QPoint(10.0, 10.0);
  Logic::GameField* game_field_ = nullptr;
};

} // View

#endif // GAMEFIELD_H
