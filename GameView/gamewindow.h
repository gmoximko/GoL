#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QQuickItem>

#include "gameview.h"

namespace View {

class GameWindow : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(int patternCount READ patternCount CONSTANT)

public:
  using QQuickItem::QQuickItem;

  Logic::SizeT patternCount() const;
  Q_INVOKABLE QVariant patternModelAt(int idx) const;
  void initialize(Logic::GameField& game_field);

private:
  GameView* game_view_ = nullptr;
  Logic::GameField* game_field_ = nullptr;
};

} // View

#endif // GAMEWINDOW_H
