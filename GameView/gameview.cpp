#include <cmath>

#include <QPainter>
#include <QQuickWindow>

#include "../Utilities/qtutilities.h"
#include "gameview.h"

namespace View {

namespace {

constexpr Qt::GlobalColor pattern_selection_color = Qt::GlobalColor::yellow;
constexpr QPoint pixels_per_cell = QPoint(10.0, 10.0);
constexpr qreal life_pixels_ratio = 0.3;
constexpr int max_cells_in_screen = 1024;
constexpr int min_cells_in_screen = 8;

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

  drawGrid(painter);
  drawLifeCells(painter);
  drawSelectedCell(painter);
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
  point = loopPos(point);
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

void GameView::drawGrid(QPainter& painter) const
{
  auto const window = size();
  auto const pixels_per_cell = pixelsPerCell();
  auto const cells = QPoint(window.width() / pixels_per_cell.x(), window.height() / pixels_per_cell.y());
  auto const offset = QPointF(std::fmod(field_offset_.x(), pixels_per_cell.x()),
                              std::fmod(field_offset_.y(), pixels_per_cell.y()));

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
    auto const size = QSizeF(pixelsPerCell().x(), pixelsPerCell().y());
    QRectF rect(cellToPixels(cell), size);
    painter.fillRect(rect, QBrush(pattern_selection_color));
  }
  else
  {
    painter.setBrush(QBrush(pattern_selection_color));
    for (auto const& point : current_pattern_->points())
    {
      drawFilledCircle(painter, game_model_->loopPos(point * trs));
    }
  }
}

void GameView::drawFilledCircle(QPainter& painter, QPoint cell) const
{
  auto const center = cellToPixels(cell) + pixelsPerCell() / 2.0;
  auto const radius = pixelsPerCell() * life_pixels_ratio;
  painter.drawEllipse(center, radius.x(), radius.y());
}

QPoint GameView::fieldSize() const
{
  return QPoint(fieldCells().x() * pixelsPerCell().x(),
                fieldCells().y() * pixelsPerCell().y());
}

QPointF GameView::pixelsPerCell() const
{
  return pixels_per_cell * field_scale_;
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
  QPointF const min_cells = pixels_per_cell * min_cells_in_screen;
  return std::min(window()->size().width() / min_cells.x(),
                  window()->size().height() / min_cells.y());
}

qreal GameView::minScale() const
{
  auto const cells = game_model_->cells();
  auto const max_cells_x = std::min(cells.x(), max_cells_in_screen);
  auto const max_cells_y = std::min(cells.y(), max_cells_in_screen);
  QPointF const max_cells(max_cells_x * pixels_per_cell.x(), max_cells_y * pixels_per_cell.y());
  return std::max(window()->size().width() / max_cells.x(),
                  window()->size().height() / max_cells.y());
}

} // View
