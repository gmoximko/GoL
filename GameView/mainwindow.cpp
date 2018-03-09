#include <QQmlEngine>
#include <QQmlContext>
#include <QScopedPointer>

#include "Utilities/qtutilities.h"
#include "gamewindow.h"
#include "mainwindow.h"

namespace View {

void MainWindow::createGameInstance(QPoint cells)
{
  Logic::GameField::Params params;
  params.cells_ = cells;
  game_field_ = std::make_unique<Logic::GameField>(params);

  QQmlEngine* engine = qmlEngine(this);
  QQmlComponent component(engine, QUrl(QStringLiteral("qrc:/GameWindow.qml")));
  auto* object = qobject_cast<QQuickItem*>(component.beginCreate(qmlContext(this)));
  object->setParentItem(this);
  object->setParent(this);

  auto* game_window = qobject_cast<GameWindow*>(object);
  Q_ASSERT(game_window != nullptr);
  game_window->initialize(*game_field_);

  component.completeCreate();
}

} // View
