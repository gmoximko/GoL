#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QQuickItem>

#include "gameview.h"

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

private:
  Logic::PatternPtr const pattern_ptr_;
};

class GameWindow : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(int patternCount READ patternCount CONSTANT)

public:
  using QQuickItem::QQuickItem;

  Logic::SizeT patternCount() const;
  Q_INVOKABLE QVariant patternModel(int idx) const;

  void initialize(Logic::GameField& game_field);

private:
  GameView* game_view_ = nullptr;
  Logic::GameField* game_field_ = nullptr;
};

} // View

Q_DECLARE_METATYPE(View::PatternModel)

#endif // GAMEWINDOW_H
