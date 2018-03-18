#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <QQuickPaintedItem>
#include <QPoint>

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

class GameView : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QPoint fieldSize READ fieldSize CONSTANT)
  Q_PROPERTY(QVariant currentPattern READ currentPattern WRITE setCurrentPattern NOTIFY currentPatternChanged)

public:
  using QQuickPaintedItem::QQuickPaintedItem;

  QPoint fieldSize() const;
  QVariant currentPattern() const;

  void initialize(QPoint cells);
  void setCurrentPattern(QVariant const& pattern_model);
  void paint(QPainter* painter_ptr) override;
  Q_INVOKABLE void pressed(QPointF point);

signals:
  void currentPatternChanged();

private:
  void drawGrid(QPainter& painter) const;
  void drawSelectedCell(QPainter& painter) const;

  QPoint cells_;

  using MaybeCell = QPair<QPoint, bool>;
  MaybeCell selected_cell_;
  Logic::PatternPtr current_pattern_;
};

} // View

Q_DECLARE_METATYPE(View::PatternModel)

#endif // GAMEFIELD_H
