#include <QQmlEngine>
#include <QQmlContext>
#include <QScopedPointer>

#include "../Utilities/qtutilities.h"
#include "mainwindow.h"

namespace View {

void MainWindow::createGame()
{
  createGameModel();
  createGameView();
  createGameController();

  connect(game_view_.data(), &GameView::patternSelected,
          [this](auto const pattern_trs){ game_controller_->addPattern(pattern_trs); });
}

void MainWindow::createGameModel()
{
  game_model_ = Logic::createGameModel({ cells_ });
}

void MainWindow::createGameController()
{
  Q_ASSERT(game_model_ != nullptr);
  game_controller_ = Logic::createGameController({ game_model_ });
}

void MainWindow::createGameView()
{
  QQmlEngine* engine = qmlEngine(this);
  QQmlComponent component(engine, QUrl(QStringLiteral("qrc:/GameWindow.qml")));
  auto* object = qobject_cast<QQuickItem*>(component.beginCreate(qmlContext(this)));
  object->setParentItem(this);
  object->setParent(this);

  game_view_ = object->findChild<GameView*>();
  Q_ASSERT(game_view_ != nullptr);
  Q_ASSERT(game_model_ != nullptr);
  game_view_->initialize(game_model_);

  component.completeCreate();
}

} // View
