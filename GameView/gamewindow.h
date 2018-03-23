#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QQuickItem>

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

class GameWindow : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(int patternCount READ patternCount CONSTANT)
  Q_PROPERTY(QVariant currentPattern READ currentPattern WRITE setCurrentPattern NOTIFY currentPatternChanged)

public:
  using QQuickItem::QQuickItem;
  using MaybeCell = QPair<QPoint, bool>;

  QPoint cells() const
  {
    return game_field_->cells();
  }
  MaybeCell selectedCell() const
  {
    return selected_cell_;
  }
  Logic::PatternPtr currentPatternPtr() const
  {
    return current_pattern_;
  }
  Logic::SizeT patternCount() const
  {
    return game_field_->allPatterns()->patternCount();
  }
  QVariant currentPattern() const;
  Q_INVOKABLE QVariant patternModelAt(int idx) const;

  void initialize(Logic::GameField& game_field);
  void setCurrentPattern(QVariant const& pattern_model);
  Q_INVOKABLE void pressed(QPointF point);

signals:
  void currentPatternChanged();

private:
  Logic::GameField* game_field_ = nullptr;
  MaybeCell selected_cell_;
  Logic::PatternPtr current_pattern_;
};

} // View

Q_DECLARE_METATYPE(View::PatternModel)

#endif // GAMEWINDOW_H
