#include <QPainter>

#include "gameview.h"

namespace View {

void GameView::paint(QPainter* painter_ptr)
{
  auto& painter = *painter_ptr;
  painter.setPen(Qt::GlobalColor::white);
  drawGrid(painter);
  drawCoordinates(painter);
  drawSelectedCell(painter);
}

void GameView::pressed(QPointF point)
{
  selected_cell_ = qMakePair(QPoint(point.x() / pixels_per_cell_.x(),
                                    point.y() / pixels_per_cell_.y()), true);
  update();
}

void GameView::drawGrid(QPainter& painter) const
{
  QPoint const field_size = fieldSize();
  QPoint const cells = gameField().cells();
  for (int x = 0; x <= cells.x(); ++x)
  {
    int line_x = x * pixels_per_cell_.x();
    painter.drawLine(line_x, 0, line_x, field_size.y());
  }
  for (int y = 0; y <= cells.y(); ++y)
  {
    int line_y = y * pixels_per_cell_.y();
    painter.drawLine(0, line_y, field_size.x(), line_y);
  }
}

void GameView::drawCoordinates(QPainter& painter) const
{
  painter.drawText(boundingRect().center(), "Hello, World!");
}

void GameView::drawSelectedCell(QPainter& painter) const
{
  if (!selected_cell_.second)
  {
    return;
  }
  QPoint cell = selected_cell_.first;
  QRectF rect(cell.x() * pixels_per_cell_.x(),
              cell.y() * pixels_per_cell_.y(),
              pixels_per_cell_.x(),
              pixels_per_cell_.y());
  painter.fillRect(rect, QBrush(Qt::GlobalColor::white));
}

} // View
