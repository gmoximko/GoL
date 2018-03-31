#include <QPainter>
#include <QTimer>

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
  return QPoint(fieldCells().x() * pixels_per_cell.x(),
                fieldCells().y() * pixels_per_cell.y());
}

QVariant GameView::currentPattern() const
{
  return current_pattern_ == nullptr
      ? QVariant()
      : QVariant::fromValue(PatternModel(current_pattern_));
}

Logic::SizeT GameView::patternCount() const
{
  return game_model_->allPatterns()->patternCount();
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

  drawGrid(painter);
  drawLifeCells(painter);
  drawSelectedCell(painter);
}

QVariant GameView::patternModelAt(int idx) const
{
  return QVariant::fromValue(PatternModel(game_model_->allPatterns()->patternAt(idx)));
}

void GameView::pressed(QPointF point)
{
  QPoint cell(point.x() / pixels_per_cell.x(),
              point.y() / pixels_per_cell.y());

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
  Q_ASSERT(current_pattern_ != nullptr);

  emit patternSelected(qMakePair(current_pattern_, std::move(pattern_trs_.second)));
  pattern_trs_ = qMakePair(false, QMatrix());
  current_pattern_ = nullptr;
  update();
}

void GameView::drawGrid(QPainter& painter) const
{
  auto const field_size = fieldSize();
  auto const cells = fieldCells();
  for (int x = 0; x <= cells.x(); ++x)
  {
    int line_x = x * pixels_per_cell.x();
    painter.drawLine(line_x, 0, line_x, field_size.y());
  }
  for (int y = 0; y <= cells.y(); ++y)
  {
    int line_y = y * pixels_per_cell.y();
    painter.drawLine(0, line_y, field_size.x(), line_y);
  }
}

void GameView::drawLifeCells(QPainter& painter) const
{
  painter.setBrush(QBrush(Qt::GlobalColor::white));
  for (auto const& unit : game_model_->lifeUnits())
  {
    drawFilledCircle(painter, unit);
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
    QRectF rect(cellToPixels(cell), QSizeF(pixels_per_cell.x(), pixels_per_cell.y()));
    painter.fillRect(rect, QBrush(pattern_selection_color));
  }
  else
  {
    painter.setBrush(QBrush(pattern_selection_color));
    for (auto const& point : current_pattern_->points())
    {
      drawFilledCircle(painter, point * trs);
    }
  }
}

void GameView::drawFilledCircle(QPainter& painter, QPoint cell) const
{
  auto const center = cellToPixels(cell) + pixels_per_cell / 2.0;
  auto const radius = pixels_per_cell * life_pixels_ratio;
  painter.drawEllipse(center, radius.x(), radius.y());
}

} // View
