#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQuickItem>

#include "GameLogic/gamelogic.h"

namespace View {

class MainWindow : public QQuickItem
{
  Q_OBJECT

public:
  using QQuickItem::QQuickItem;
  Q_INVOKABLE void createGameInstance(QPoint cells);

private:
  Logic::GameFieldPtr game_field_;
};

}

#endif // MAINWINDOW_H
