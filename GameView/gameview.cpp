#include <cmath>

#include <QPainter>
#include <QQuickWindow>

#include "../Utilities/qtutilities.h"
#include "gameview.h"

namespace View {

namespace {

constexpr auto const c_pattern_selection_color = Qt::GlobalColor::yellow;
constexpr auto const c_pixels_per_cell = QPoint(10.0, 10.0);
constexpr auto const c_life_pixels_ratio = 0.3;
constexpr auto const c_max_cells_in_screen = 1024;
constexpr auto const c_min_cells_in_screen = 8;
constexpr auto const c_scale_to_hide_grid = 0.2;

template<typename T>
T normalizeValue(T value, T min, T max)
{
  return (value + min) / (min + max);
}

QColor playerColor(uint8_t player)
{
  switch (player)
  {
  case 0: return Qt::GlobalColor::red;
  case 1: return Qt::GlobalColor::blue;
  case 2: return Qt::GlobalColor::green;
  case 3:
  default: return Qt::GlobalColor::cyan;
  }
}

} // namespace

QVariant GameView::currentPattern() const
{
  return current_pattern_ == nullptr
      ? QVariant()
      : QVariant::fromValue(PatternModel(current_pattern_));
}

Logic::SizeT GameView::patternCount() const
{
  return game_model_->patternCount();
}

QPoint GameView::fieldCells() const
{
  return game_model_->cells();
}

void GameView::initialize(Logic::GameModelPtr game_model)
{
  game_model_ = game_model;
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

  painter.save();
  drawLifeCells(painter);
  painter.restore();

  painter.save();
  drawSelectedCell(painter);
  painter.restore();

  auto const normalized_scale = normalizeValue(field_scale_, minScale(), maxScale());
  if (normalized_scale < c_scale_to_hide_grid)
  {
    return;
  }
  else if (normalized_scale < 1.0)
  {
    painter.setOpacity(normalized_scale);
  }
  drawGrid(painter);
  drawCoordinates(painter);
}

void GameView::setFieldOffset(QPointF field_offset)
{
  field_offset_ = loopPos(field_offset);
  update();
}

void GameView::setFieldScale(qreal field_scale)
{
  field_scale_ = field_scale;
  update();
}

QVariant GameView::patternModelAt(int idx) const
{
  return QVariant::fromValue(PatternModel(game_model_->patternAt(idx)));
}

void GameView::pressed(QPointF point)
{
  point = loopPos(point - field_offset_);
  QPoint cell(point.x() / pixelsPerCell().x(),
              point.y() / pixelsPerCell().y());

  pattern_trs_.first = true;
  auto& trs = pattern_trs_.second;
  QMatrix tmp;
  tmp.translate(cell.x() - trs.dx(), cell.y() - trs.dy());
  trs *= tmp;
  update();
}

void GameView::rotatePattern(qreal angle)
{
  if (!pattern_trs_.first)
  {
    return;
  }
  auto& trs = pattern_trs_.second;
  trs.rotate(angle);
  update();
}

void GameView::selectPattern()
{
  Q_ASSERT(pattern_trs_.first);
  class SingleCell : public Logic::Pattern
  {
  public:
    QString name() const override
    {
      return "";
    }
    Logic::Points const& points() const override
    {
      return points_;
    }
    QPoint size() const override
    {
      return QPoint(1, 1);
    }
    Logic::SizeT scores() const override
    {
      return points_.size();
    }

  private:
    Logic::Points const points_ { QPoint() };
  };

  if (current_pattern_ == nullptr)
  {
    current_pattern_ = Utilities::Qt::makeShared<SingleCell>();
  }
  emit patternSelected(qMakePair(current_pattern_, std::move(pattern_trs_.second)));
  pattern_trs_ = qMakePair(false, QMatrix());
  current_pattern_ = nullptr;
  update();
}

void GameView::zoom(qreal ratio, QPointF point)
{
  auto const max_scale = maxScale();
  auto const min_scale = minScale();
  auto new_scale = field_scale_ + ratio;
  if (new_scale > max_scale)
  {
    ratio = max_scale - field_scale_;
    new_scale = max_scale;
  }
  else if (new_scale < min_scale)
  {
    ratio = min_scale - field_scale_;
    new_scale = min_scale;
  }
  Q_ASSERT(field_scale_ + ratio <= max_scale);
  Q_ASSERT(field_scale_ + ratio >= min_scale);

  auto const world_point = loopPos(point - field_offset_);
  auto const old_size = fieldSize();
  QPointF const normalized_point(world_point.x() / old_size.x(), world_point.y() / old_size.y());
  field_scale_ = new_scale;

  auto const new_size = fieldSize();
  setFieldOffset(-QPointF(normalized_point.x() * new_size.x(),
                          normalized_point.y() * new_size.y()) + point);
}

void GameView::drawGrid(QPainter& painter) const
{
  auto const window = size();
  auto const pixels_per_cell = pixelsPerCell();
  auto const cells = cellsOnTheScreen();
  auto const offset = cellOffset();

  for (int x = 0; x <= cells.x(); ++x)
  {    
    qreal line_x = x * pixels_per_cell.x() + offset.x();
    painter.drawLine(QLineF(line_x, 0, line_x, window.height()));
  }
  for (int y = 0; y <= cells.y(); ++y)
  {
    qreal line_y = y * pixels_per_cell.y() + offset.y();
    painter.drawLine(QLineF(0, line_y, window.width(), line_y));
  }
}

void GameView::drawLifeCells(QPainter& painter) const
{
  painter.setBrush(QBrush(Qt::GlobalColor::white));
  for (auto const unit : game_model_->lifeUnits())
  {
    painter.setPen(playerColor(unit.player()));
    drawFilledCircle(painter, QPoint(unit.x(), unit.y()));
  }
}

void GameView::drawSelectedCell(QPainter& painter) const
{
  if (!pattern_trs_.first)
  {
    return;
  }
  auto const& trs = pattern_trs_.second;

  if (current_pattern_ == nullptr)
  {
    auto const cell = QPoint(trs.dx(), trs.dy());
    auto const size = QSizeF(pixelsPerCell().x(), pixelsPerCell().y());
    QRectF rect(cellToPixels(cell), size);
    painter.fillRect(rect, QBrush(c_pattern_selection_color));
  }
  else
  {
    painter.setPen(c_pattern_selection_color);
    for (auto const& point : current_pattern_->points())
    {
      drawFilledCircle(painter, Logic::loopPos(point * trs, game_model_->cells()));
    }
  }
}

void GameView::drawFilledCircle(QPainter& painter, QPoint cell) const
{
  auto const center = cellToPixels(cell) + pixelsPerCell() / 2.0;
  auto const radius = pixelsPerCell() * c_life_pixels_ratio;
  painter.drawEllipse(center, radius.x(), radius.y());
}

void GameView::drawCoordinates(QPainter& painter) const
{
  auto const window = size();
  auto const cells = cellsOnTheScreen();
  auto const offset = cellOffset();
  auto const cell_center = pixelsPerCell() / 2;
  auto const font_metrics = painter.fontMetrics();
  auto const field_cells = fieldCells();

  QPoint numbers;
  for (int x = -1; x <= cells.x(); ++x)
  {
    auto const pos_x = x * pixelsPerCell().x() + cell_center.x() + offset.x();
    numbers.setX(x + static_cast<int>(field_offset_.x() / -pixelsPerCell().x()));
    numbers = Logic::loopPos(numbers, field_cells);
    auto const number_str = QString::number(numbers.x());

    QPointF top(pos_x, font_metrics.height());
    QPointF bottom(pos_x, window.height());
    painter.drawText(top, number_str);
    painter.drawText(bottom, number_str);
  }
  for (int y = -1; y <= cells.y(); ++y)
  {
    auto const pos_y = y * pixelsPerCell().y() + cell_center.y() + offset.y();
    numbers.setY(y + static_cast<int>(field_offset_.y() / -pixelsPerCell().y()));
    numbers = Logic::loopPos(numbers, field_cells);
    auto const number_str = QString::number(numbers.y());

    QPointF left(0, pos_y);
    QPointF right(window.width() - font_metrics.width(number_str), pos_y);
    painter.drawText(left, number_str);
    painter.drawText(right, number_str);
  }
}

QPoint GameView::fieldSize() const
{
  return QPoint(fieldCells().x() * pixelsPerCell().x(),
                fieldCells().y() * pixelsPerCell().y());
}

QPoint GameView::cellsOnTheScreen() const
{
  return QPoint(size().width() / pixelsPerCell().x(), size().height() / pixelsPerCell().y());
}

QPointF GameView::cellOffset() const
{
  return QPointF(std::fmod(field_offset_.x(), pixelsPerCell().x()),
                 std::fmod(field_offset_.y(), pixelsPerCell().y()));
}

QPointF GameView::pixelsPerCell() const
{
  return c_pixels_per_cell * field_scale_;
}

QPointF GameView::cellToPixels(QPoint cell) const
{
  QPointF const point(cell.x() * pixelsPerCell().x(),
                      cell.y() * pixelsPerCell().y());
  return loopPos(point + field_offset_);
}

QPointF GameView::loopPos(QPointF point) const
{
  QPointF const field_size = fieldSize();
  Q_ASSERT(field_size != QPointF());
  return QPointF(std::fmod((point.x() + field_size.x()), field_size.x()),
                 std::fmod((point.y() + field_size.y()), field_size.y()));
}

qreal GameView::maxScale() const
{
  QPointF const min_cells = c_pixels_per_cell * c_min_cells_in_screen;
  return std::min(window()->size().width() / min_cells.x(),
                  window()->size().height() / min_cells.y());
}

qreal GameView::minScale() const
{
  auto const cells = game_model_->cells();
  auto const max_cells_x = std::min(cells.x(), c_max_cells_in_screen);
  auto const max_cells_y = std::min(cells.y(), c_max_cells_in_screen);
  QPointF const max_cells(max_cells_x * c_pixels_per_cell.x(), max_cells_y * c_pixels_per_cell.y());
  return std::max(window()->size().width() / max_cells.x(),
                  window()->size().height() / max_cells.y());
}

} // View
