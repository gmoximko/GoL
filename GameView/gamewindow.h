#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QQuickItem>

#include "Utilities/qtutilities.h"
#include "gameview.h"

namespace View {

class GameWindow : public QQuickItem
{
  Q_OBJECT

public:
  using QQuickItem::QQuickItem;

  void initialize(Logic::GameField& game_field);

private:
  template<class T>
  using NotNullPtr = Utilities::Qt::NotNullPtr<T>;
  NotNullPtr<GameView> game_view_;
  NotNullPtr<Logic::GameField> game_field_;
};

} // View

#endif // GAMEWINDOW_H
