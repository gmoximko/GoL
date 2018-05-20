#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQuickItem>

#include "../GameLogic/gamemodel.h"
#include "../GameLogic/gamecontroller.h"
#include "gameview.h"

namespace View {

class MainWindow : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QPoint cells MEMBER cells_)

public:
  using QQuickItem::QQuickItem;
  Q_INVOKABLE QQuickItem* createGame();
  Q_INVOKABLE bool destroyGame();

private:
  void createGameModel();
  void createGameController();
  void createGameView();

  Logic::GameModelMutablePtr game_model_;
  Logic::GameControllerPtr game_controller_;
  View::GameViewPtr game_view_;
  QScopedPointer<QQuickItem> game_window_;

  // Game parameters
  QPoint cells_;
};

}

#endif // MAINWINDOW_H
