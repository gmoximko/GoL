#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QQuickPaintedItem>
#include <QPoint>
#include <QMatrix>
#include <QPointer>

#include "../GameLogic/gamemodel.h"

namespace View {

class PatternModel
{
  Q_GADGET
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QPoint size READ size CONSTANT)
  Q_PROPERTY(int scores READ scores CONSTANT)

public:
  PatternModel() = default;
  explicit PatternModel(Logic::PatternPtr const pattern_ptr)
    : pattern_ptr_(pattern_ptr)
  {}

  QString name() const
  {
    return pattern_ptr_->name();
  }
  QPoint size() const
  {
    return pattern_ptr_->size();
  }
  Logic::SizeT scores() const
  {
    return pattern_ptr_->scores();
  }
  Logic::PatternPtr pattern() const
  {
    return pattern_ptr_;
  }

private:
  Logic::PatternPtr pattern_ptr_;
};

class GameView : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QVariant currentPattern READ currentPattern WRITE setCurrentPattern NOTIFY currentPatternChanged)
  Q_PROPERTY(int patternCount READ patternCount CONSTANT)
  Q_PROPERTY(QPointF fieldOffset MEMBER field_offset_ WRITE setFieldOffset)
  Q_PROPERTY(qreal fieldScale MEMBER field_scale_ WRITE setFieldScale)
  Q_PROPERTY(qreal maxScale READ maxScale CONSTANT)
  Q_PROPERTY(qreal minScale READ minScale CONSTANT)

public:
  using QQuickPaintedItem::QQuickPaintedItem;

  QVariant currentPattern() const;
  Logic::SizeT patternCount() const;
  QPoint fieldCells() const;

  void initialize(Logic::GameModelPtr game_model);
  void setCurrentPattern(QVariant const& pattern_model);
  void paint(QPainter* painter_ptr) override;
  void setFieldOffset(QPointF field_offset);
  void setFieldScale(qreal ratio);

  Q_INVOKABLE QVariant patternModelAt(int idx) const;
  Q_INVOKABLE void pressed(QPointF point);
  Q_INVOKABLE void rotatePattern(qreal angle);
  Q_INVOKABLE void selectPattern();
  Q_INVOKABLE void zoom(qreal ratio, QPointF point);

signals:
  void currentPatternChanged();
  void patternSelected(Logic::PatternTrs pattern_trs);

private:
  using MaybeTRS = QPair<bool, QMatrix>;
  void drawGrid(QPainter& painter) const;
  void drawLifeCells(QPainter& painter) const;
  void drawSelectedCell(QPainter& painter) const;
  void drawFilledCircle(QPainter& painer, QPoint cell) const;
  void drawCoordinates(QPainter& painter) const;

  QPointF fieldSize() const;
  QPoint cellsOnTheScreen() const;
  QPointF cellOffset() const;
  QPointF pixelsPerCell() const;
  QPointF cellToPixels(QPoint cell) const;
  QPointF loopPos(QPointF point) const;
  qreal maxScale() const;
  qreal minScale() const;

  QPointF field_offset_;
  qreal field_scale_ = 1.0;
  MaybeTRS pattern_trs_;
  Logic::PatternPtr current_pattern_;
  Logic::GameModelPtr game_model_;
};
using GameViewPtr = QPointer<GameView>;

} // View

Q_DECLARE_METATYPE(View::PatternModel)

#endif // GAMEVIEW_H
