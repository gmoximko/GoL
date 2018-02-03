#include <QPainter>

#include "gameview.h"

namespace View {

void GameView::paint(QPainter* painter)
{
  painter->setPen(Qt::GlobalColor::white);
  drawGrid(painter);
  drawCoordinates(painter);
}

void GameView::pressed(QPointF point)
{
  qDebug() << QPoint(point.x() / pixels_per_cell_.x(), point.y() / pixels_per_cell_.y());
}

void GameView::drawGrid(QPainter* painter) const
{
  QPoint const field_size = fieldSize();
  QPoint const cells = gameField().cells();
  for (int x = 0; x <= cells.x(); ++x)
  {
    int line_x = x * pixels_per_cell_.x();
    painter->drawLine(line_x, 0, line_x, field_size.y());
  }
  for (int y = 0; y <= cells.y(); ++y)
  {
    int line_y = y * pixels_per_cell_.y();
    painter->drawLine(0, line_y, field_size.x(), line_y);
  }
}

void GameView::drawCoordinates(QPainter* painter) const
{
  painter->drawText(boundingRect().center(), "Hello, World!");
}

} // View
