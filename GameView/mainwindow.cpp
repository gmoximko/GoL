#include <QQmlEngine>
#include <QQmlContext>
#include <QScopedPointer>

#include "../Utilities/qtutilities.h"
#include "mainwindow.h"

namespace View {

MainWindow::MainWindow(QQuickItem* parent)
  : QQuickItem(parent)
{
  try
  {
    game_network_ = Network::createGameNetwork(this, {});
    if (game_network_ == nullptr)
    {
      emit qmlEngine(this)->quit();
    }
    else
    {
      connect(game_network_.data(), &Network::GameNetwork::lobbiesUpdated, [this](Network::Lobbies lobbies)
      {
        auto* root_context = qmlEngine(this)->rootContext();
        root_context->setContextProperty("lobbyList", QVariant::fromValue(lobbies));
      });
    }
  }
  catch(std::exception const& e)
  {
    qDebug() << e.what();
  }
}

QQuickItem* MainWindow::createGame()
{
  createGameModel();
  createGameView();
  createGameController();

  connect(game_view_.data(), &GameView::patternSelected,
          game_controller_.data(), &Logic::GameController::addPattern);
  connect(game_controller_.data(), &Logic::GameController::stepMade, [this]
  {
    game_view_->update();
  });
  return game_window_.data();
}

bool MainWindow::destroyGame()
{
  auto const result = game_window_.isNull();
  game_model_ = nullptr;
  game_window_.reset(nullptr);
  Q_ASSERT(game_controller_.isNull());
  Q_ASSERT(game_view_.isNull());
  return !result;
}

void MainWindow::createGameModel()
{
  game_model_ = Logic::createGameModel({ cells_ });
}

void MainWindow::createGameController()
{
  Q_ASSERT(game_model_ != nullptr);
  Q_ASSERT(game_window_ != nullptr);
  game_controller_ = Logic::createGameController(game_window_.data(), { game_model_ });
}

void MainWindow::createGameView()
{
  QQmlEngine* engine = qmlEngine(this);
  QQmlComponent component(engine, QUrl(QStringLiteral("qrc:/gamewindow.qml")));
  auto* object = qobject_cast<QQuickItem*>(component.beginCreate(qmlContext(this)));
  QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
  object->setParentItem(this);
  object->setParent(this);

  game_view_ = object->findChild<GameView*>();
  Q_ASSERT(game_view_ != nullptr);
  Q_ASSERT(game_model_ != nullptr);
  game_view_->initialize(game_model_);
  game_window_.reset(object);

  component.completeCreate();
}

} // View
