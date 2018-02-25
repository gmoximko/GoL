#include <QQmlEngine>
#include <QQmlContext>
#include <QScopedPointer>

#include "gameview.h"
#include "mainwindow.h"

namespace View {

void MainWindow::createGameInstance(QPoint cells)
{
  Logic::GameField::Params params;
  params.cells_ = cells;
  game_field_ = std::make_unique<Logic::GameField>(params);

  QQmlEngine* engine = qmlEngine(this);
  QQmlComponent component(engine, QUrl(QStringLiteral("qrc:/GameView.qml")));
  auto* object = qobject_cast<QQuickItem*>(component.beginCreate(qmlContext(this)));
  object->setParentItem(this);
  object->setParent(this);

  auto* game_view = object->findChild<GameView*>();
  Q_ASSERT(game_view != nullptr);
  game_view->setGameField(game_field_.get());
  component.completeCreate();
}

} // View
