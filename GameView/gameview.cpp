#include <cmath>
#include <algorithm>
#include <random>

#include <QtMath>
#include <QPainter>
#include <QQuickWindow>
#include <QSettings>

#include "../Utilities/qtutilities.h"
#include "gameview.h"

namespace View {

namespace {

constexpr auto c_dark_theme_color = Qt::GlobalColor::black;
constexpr auto c_light_theme_color = Qt::GlobalColor::white;
constexpr auto c_pattern_selection_color = Qt::GlobalColor::yellow;
constexpr auto c_pixels_per_cell = QPointF(3.0, 3.0);
constexpr auto c_life_pixels_ratio = 0.3;
constexpr auto c_max_cells_in_screen = 4096;
constexpr auto c_min_cells_in_screen = 128;
constexpr auto c_scale_to_hide_grid = 0.5;

void flip(QTransform& matrix)
{
  matrix.scale(-1, 1);
}

void rotate(QTransform& matrix, qreal angle)
{
  matrix.rotate(angle);
}

auto makeRandomColors(std::mt19937& mt)
{
  std::array<QColor, 15> result
  {
    Qt::GlobalColor::red,
    Qt::GlobalColor::blue,
    Qt::GlobalColor::cyan,
    Qt::GlobalColor::gray,
    Qt::GlobalColor::green,
    Qt::GlobalColor::magenta,
    Qt::GlobalColor::darkRed,
    Qt::GlobalColor::darkBlue,
    Qt::GlobalColor::darkCyan,
    Qt::GlobalColor::darkBlue,
    Qt::GlobalColor::darkGray,
    Qt::GlobalColor::darkGreen,
    Qt::GlobalColor::darkYellow,
    Qt::GlobalColor::darkMagenta,
    Qt::GlobalColor::lightGray,
  };
  std::shuffle(result.begin(), result.end(), mt);
  return result;
}

} // namespace

GameView::~GameView()
{
  qDebug() << "~GameView()";
}

Logic::Serializable::SavedData GameView::serialize() const
{
  SavedData data;
  data["fieldOffset"] = field_offset_;
  data["fieldScale"] = field_scale_;
  data["seed"] = seed_;
  return data;
}

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
  if (seed_ == 0)
  {
#if defined(QT_DEBUG)
    seed_ = std::mt19937::default_seed;
#else
    std::srand(std::time(nullptr));
    seed_ = std::rand() % RAND_MAX + 1;
#endif
  }

  std::random_device device;
  std::mt19937 mt(device());
  mt.seed(seed_);
  colors_ = makeRandomColors(mt);

  game_model_ = game_model;
  setDarkTheme(QSettings().value("darkTheme", true).toBool());
  setRenderTarget(RenderTarget::FramebufferObject);
//  setPerformanceHints(QQuickPaintedItem::FastFBOResizing);
}

void GameView::initialize(Logic::GameModelPtr game_model, SavedData const& data)
{
  field_offset_ = data["fieldOffset"].toPointF();
  field_scale_ = data["fieldScale"].toReal();
  seed_ = data["seed_"].toUInt();

  initialize(game_model);
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
  drawLifeCells();
  drawSelectedCell();

  auto& painter = *painter_ptr;
  painter.setPen(Qt::GlobalColor::white);
//  painter.setRenderHint(QPainter::Antialiasing);
  painter.drawImage(boundingRect(), image_);

  return;
  auto const min_scale = minScale();
  auto const max_scale = maxScale();
  auto const normalized_scale = Utilities::Qt::normalized(
        Utilities::Qt::clamp(field_scale_, min_scale, max_scale),
        min_scale,
        max_scale);
  if (normalized_scale < c_scale_to_hide_grid)
  {
    return;
  }
  else if (normalized_scale < 1.0)
  {
    painter.setOpacity(normalized_scale);
  }
  painter.setPen(darkTheme() ? c_light_theme_color : c_dark_theme_color);
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

void GameView::setDarkTheme(bool dark_theme)
{
  auto const color = dark_theme ? c_dark_theme_color : c_light_theme_color;
  setFillColor(color);
  QSettings().setValue("darkTheme", dark_theme);
  emit darkThemeChanged();
}

QVariant GameView::patternModelAt(int idx) const
{
  return QVariant::fromValue(PatternModel(game_model_->patternAt(idx)));
}

void GameView::pressed(QPointF point)
{
  auto const cell = screenToWorld(point);
  if (!pattern_trs_)
  {
    pattern_trs_ = QTransform();
  }
  auto& trs = *pattern_trs_;
  QTransform tmp;
  tmp.translate(cell.x() - trs.dx(), cell.y() - trs.dy());
  trs *= tmp;
  update();
}

void GameView::unpress()
{
  pattern_trs_.reset();
  current_pattern_ = nullptr;
  update();
}

void GameView::rotatePattern(qreal angle)
{
  if (pattern_trs_)
  {
    auto const should_flip = pattern_trs_->determinant() < 0;
    if (should_flip)
    {
      flip(*pattern_trs_);
    }
    rotate(*pattern_trs_, angle);
    if (should_flip)
    {
      flip(*pattern_trs_);
    }
    update();
  }
}

void GameView::flipPattern()
{
  if (pattern_trs_)
  {
    auto const angle = std::abs(qRadiansToDegrees(std::acos(pattern_trs_->m11())));
    auto const should_rotate = qFuzzyCompare(angle, 90) || qFuzzyCompare(angle, 270);
    if (should_rotate)
    {
      rotate(*pattern_trs_, 90);
    }
    flip(*pattern_trs_);
    if (should_rotate)
    {
      rotate(*pattern_trs_, -90);
    }
    update();
  }
}

void GameView::selectPattern()
{
  class SingleCell final : public Logic::Pattern
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
    Logic::Score scores() const override
    {
      return static_cast<Logic::Score>(points_.size());
    }

  private:
    Logic::Points const points_ { QPoint() };
  };

  Q_ASSERT(pattern_trs_);
  if (current_pattern_ == nullptr)
  {
    current_pattern_ = Utilities::Qt::makeShared<SingleCell>();
  }
  emit patternSelected(qMakePair(current_pattern_, std::move(*pattern_trs_)));
  pattern_trs_.reset();
  current_pattern_ = nullptr;
  emit currentPatternChanged();
  update();
}

void GameView::zoom(qreal ratio, QPointF point)
{
  auto const max_scale = maxScale();
  auto const min_scale = minScale();
  auto new_scale = ratio;
  if (new_scale > max_scale)
  {
    new_scale = max_scale;
  }
  else if (new_scale < min_scale)
  {
    new_scale = min_scale;
  }

  auto const world_point = loopPos(point - field_offset_);
  auto const old_size = fieldSize();
  QPointF const normalized_point(world_point.x() / old_size.x(), world_point.y() / old_size.y());
  field_scale_ = new_scale;

  auto const new_size = fieldSize();
  setFieldOffset(-QPointF(normalized_point.x() * new_size.x(),
                          normalized_point.y() * new_size.y()) + point);
}

void GameView::onStepMade(Logic::Score scores)
{
  if (scores != scores_)
  {
    scores_ = scores;
    emit scoresChanged();
  }
  update();
}

void GameView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
  image_ = QImage(width(), height(), QImage::Format_RGB32);
  QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
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

void GameView::drawLifeCells()
{
  image_.fill(darkTheme() ? c_dark_theme_color : c_light_theme_color);

  auto const rect = boundingRect();
  auto const visible_world = QRect(screenToWorld(rect.topLeft()),
                                   QSize(rect.width() / pixelsPerCell().x(),
                                         rect.height() / pixelsPerCell().y()));
  for (auto const unit : game_model_->lifeUnits(visible_world))
  {
    drawLifeUnit(QPoint(unit.x(), unit.y()));
  }
}

void GameView::drawSelectedCell()
{
  if (!pattern_trs_)
  {
    return;
  }
  auto const& trs = *pattern_trs_;

  if (current_pattern_ == nullptr)
  {
    auto const cell = QPoint(static_cast<int>(trs.dx()), static_cast<int>(trs.dy()));
    auto const size = QSizeF(pixelsPerCell().x(), pixelsPerCell().y());
    QRectF rect(worldToScreen(cell), size);
    QPainter(&image_).fillRect(rect, QBrush(c_pattern_selection_color));
  }
  else
  {
    for (auto const& point : current_pattern_->points())
    {
      drawSelectedUnit(Logic::loopPos(point * trs, game_model_->cells()));
    }
  }
}

void GameView::drawSelectedUnit(QPoint cell)
{
  auto const center = worldToScreen(cell) + pixelsPerCell() / 2.0;
  if (boundingRect().contains(center))
  {
    drawUnitPixel(center, c_pattern_selection_color);
  }
}

void GameView::drawLifeUnit(QPoint cell)
{
  auto const center = worldToScreen(cell) + pixelsPerCell() / 2.0;
  if (boundingRect().contains(center))
  {
    auto const hash = qHash(qMakePair(cell.x(), cell.y()));
    drawUnitPixel(center, colors_[hash % colors_.size()]);
  }
}

void GameView::drawUnitPixel(QPointF center, QColor const& color)
{
  auto const x = static_cast<int>(center.x());
  auto const y = static_cast<int>(center.y());
  auto* rgb = reinterpret_cast<QRgb*>(image_.bits());
  rgb[x + y * image_.width()] = color.rgb();
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
    QPointF right(window.width() - font_metrics.horizontalAdvance(number_str), pos_y);
    painter.drawText(left, number_str);
    painter.drawText(right, number_str);
  }
}

QPointF GameView::fieldSize() const
{
  return QPointF(fieldCells().x() * pixelsPerCell().x(),
                 fieldCells().y() * pixelsPerCell().y());
}

QPoint GameView::cellsOnTheScreen() const
{
  return QPoint(static_cast<int>(size().width() / pixelsPerCell().x()),
                static_cast<int>(size().height() / pixelsPerCell().y()));
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

QPointF GameView::worldToScreen(QPoint world) const
{
  QPointF const point(world.x() * pixelsPerCell().x(),
                      world.y() * pixelsPerCell().y());
  return loopPos(point + field_offset_);
}

QPoint GameView::screenToWorld(QPointF screen) const
{
  screen = loopPos(screen - field_offset_);
  return QPoint(static_cast<int>(screen.x() / pixelsPerCell().x()),
                static_cast<int>(screen.y() / pixelsPerCell().y()));
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
  auto const min_cells = c_pixels_per_cell * c_min_cells_in_screen;
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

bool GameView::darkTheme() const
{
  return fillColor() == c_dark_theme_color;
}

} // View
