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
  Q_PROPERTY(QPointF pixelsPerCell READ pixelsPerCell CONSTANT)
  Q_PROPERTY(QPoint fieldSize READ fieldSize CONSTANT)
  Q_PROPERTY(QVariant currentPattern READ currentPattern WRITE setCurrentPattern NOTIFY currentPatternChanged)
  Q_PROPERTY(int patternCount READ patternCount CONSTANT)

public:
  using QQuickPaintedItem::QQuickPaintedItem;

  QPointF pixelsPerCell() const;
  QPoint fieldSize() const;
  QVariant currentPattern() const;
  Logic::SizeT patternCount() const;
  QPoint fieldCells() const;

  void initialize(Logic::GameModelPtr const game_model);
  void setCurrentPattern(QVariant const& pattern_model);
  void paint(QPainter* painter_ptr) override;

  Q_INVOKABLE QVariant patternModelAt(int idx) const;
  Q_INVOKABLE void pressed(QPointF point);
  Q_INVOKABLE void rotatePattern(qreal angle);
  Q_INVOKABLE void selectPattern();

signals:
  void currentPatternChanged();
  void patternSelected(Logic::PatternTrs pattern_trs);

private:
  using MaybeTRS = QPair<bool, QMatrix>;
  void drawGrid(QPainter& painter) const;
  void drawSelectedCell(QPainter& painter) const;

  MaybeTRS pattern_trs_;
  Logic::PatternPtr current_pattern_;
  Logic::GameModelPtr game_model_;
};
using GameViewPtr = QPointer<GameView>;

} // View

Q_DECLARE_METATYPE(View::PatternModel)

#endif // GAMEVIEW_H
