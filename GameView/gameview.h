#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <QQuickPaintedItem>
#include <QPoint>

#include "GameLogic/gamelogic.h"

namespace View {

class GameView : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QPoint fieldSize READ fieldSize CONSTANT)

public:
  using QQuickPaintedItem::QQuickPaintedItem;

  QPoint fieldSize() const
  {
    return QPoint(cells_.x() * pixels_per_cell_.x(),
                  cells_.y() * pixels_per_cell_.y());
  }

  void initialize(QPoint cells);
  void paint(QPainter* painter_ptr) override;
  Q_INVOKABLE void pressed(QPointF point);

private:
  void drawGrid(QPainter& painter) const;
  void drawSelectedCell(QPainter& painter) const;

  QPoint const pixels_per_cell_ = QPoint(10.0, 10.0);
  QPoint cells_;

  using MaybeCell = QPair<QPoint, bool>;
  MaybeCell selected_cell_;
};

} // View

#endif // GAMEFIELD_H
