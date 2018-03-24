#include <QPainter>

#include "gameview.h"

namespace View {

namespace {

constexpr Qt::GlobalColor pattern_selection_color = Qt::GlobalColor::yellow;
constexpr QPoint pixels_per_cell = QPoint(10.0, 10.0);
constexpr qreal life_pixels_ratio = 0.3;

QPointF cellToPixels(QPoint cell)
{
  return QPointF(cell.x() * pixels_per_cell.x(), cell.y() * pixels_per_cell.y());
}

} // namespace

QPointF GameView::pixelsPerCell() const
{
  return pixels_per_cell;
}

QPoint GameView::fieldSize() const
{
  return QPoint(cells_.x() * pixels_per_cell.x(),
                cells_.y() * pixels_per_cell.y());
}

QVariant GameView::currentPattern() const
{
  return QVariant::fromValue(PatternModel(current_pattern_));
}

void GameView::initialize(QPoint cells)
{
  cells_ = cells;
}

void GameView::setCurrentPattern(QVariant const& pattern_model)
{
  auto const model = pattern_model.value<PatternModel>();
  Q_ASSERT(model.pattern() != nullptr);
  if (current_pattern_ != model.pattern())
  {
    current_pattern_ = model.pattern();
    emit currentPatternChanged();
    update();
  }
}

void GameView::paint(QPainter* painter_ptr)
{
  auto& painter = *painter_ptr;
  painter.setPen(Qt::GlobalColor::white);
  painter.setRenderHint(QPainter::Antialiasing);

  drawGrid(painter);
  drawSelectedCell(painter);
}

void GameView::pressed(QPointF point)
{
  selected_cell_ = qMakePair(QPoint(point.x(), point.y()), true);
  update();
}

void GameView::drawGrid(QPainter& painter) const
{
  QPoint const field_size = fieldSize();
  for (int x = 0; x <= cells_.x(); ++x)
  {
    int line_x = x * pixels_per_cell.x();
    painter.drawLine(line_x, 0, line_x, field_size.y());
  }
  for (int y = 0; y <= cells_.y(); ++y)
  {
    int line_y = y * pixels_per_cell.y();
    painter.drawLine(0, line_y, field_size.x(), line_y);
  }
}

void GameView::drawSelectedCell(QPainter& painter) const
{
  if (!selected_cell_.second)
  {
    return;
  }
  auto const cell = selected_cell_.first;

  if (current_pattern_ == nullptr)
  {
    QRectF rect(cellToPixels(cell), QSizeF(pixels_per_cell.x(), pixels_per_cell.y()));
    painter.fillRect(rect, QBrush(pattern_selection_color));
  }
  else
  {
    painter.setBrush(QBrush(pattern_selection_color));
    for (auto const& point : current_pattern_->points())
    {
      auto const center = cellToPixels(cell + point) + pixels_per_cell / 2.0f;
      auto const radius = pixels_per_cell * life_pixels_ratio;
      painter.drawEllipse(center, radius.x(), radius.y());
    }
  }
}

} // View
