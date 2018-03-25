#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <QQuickPaintedItem>
#include <QPoint>
#include <QMatrix>

#include "../GameLogic/gamelogic.h"

namespace View {

class PatternModel
{
  Q_GADGET
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QPoint size READ size CONSTANT)
  Q_PROPERTY(int scores READ scores CONSTANT)

public:
  PatternModel() = default;
  PatternModel(Logic::PatternPtr const pattern_ptr)
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

using PatternTRS = QPair<Logic::PatternPtr, QMatrix> const;

class GameView : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QPointF pixelsPerCell READ pixelsPerCell CONSTANT)
  Q_PROPERTY(QPoint fieldSize READ fieldSize CONSTANT)
  Q_PROPERTY(QVariant currentPattern READ currentPattern WRITE setCurrentPattern NOTIFY currentPatternChanged)

public:
  using QQuickPaintedItem::QQuickPaintedItem;

  QPointF pixelsPerCell() const;
  QPoint fieldSize() const;
  QVariant currentPattern() const;

  void initialize(QPoint cells);
  void setCurrentPattern(QVariant const& pattern_model);
  void paint(QPainter* painter_ptr) override;
  Q_INVOKABLE void pressed(QPointF point);
  Q_INVOKABLE void rotatePattern(qreal angle);
  Q_INVOKABLE void selectPattern();

signals:
  void currentPatternChanged();
  void patternSelected(PatternTRS pattern_trs);

private:
  using MaybeTRS = QPair<bool, QMatrix>;
  void drawGrid(QPainter& painter) const;
  void drawSelectedCell(QPainter& painter) const;

  QPoint cells_;
  MaybeTRS pattern_trs_;
  Logic::PatternPtr current_pattern_;
};

} // View

Q_DECLARE_METATYPE(View::PatternModel)

#endif // GAMEFIELD_H
