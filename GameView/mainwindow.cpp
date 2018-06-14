#include <QQmlEngine>
#include <QQmlContext>
#include <QScopedPointer>
#include <QGuiApplication>

#include "../Utilities/qtutilities.h"
#include "mainwindow.h"

namespace View {

MainWindow::MainWindow(QQuickItem* parent)
  : QQuickItem(parent)
{
  try
  {
    game_network_ = Network::createGameNetwork(this);
    if (game_network_ == nullptr)
    {
      QGuiApplication::quit();
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

QQuickItem* MainWindow::createGame(GameParams* params)
{
  Q_ASSERT(params != nullptr);
  createGameModel(*params);
  createGameView(*params);
  createGameController(*params);

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

void MainWindow::createLobby(GameParams* params)
{
  connect(game_network_.data(), &Network::GameNetwork::lobbyCreated,
          params, &GameParams::setLobby, Qt::UniqueConnection);
  game_network_->createLobby(params->getParams());
}

void MainWindow::joinLobby(GameParams* params)
{
  connect(game_network_.data(), &Network::GameNetwork::lobbyCreated,
          params, &GameParams::setLobby, Qt::UniqueConnection);
  game_network_->joinLobby(params->getParams());
}

void MainWindow::createGameModel(GameParams const& params)
{
  game_model_ = Logic::createGameModel({ params.fieldSize() });
}

void MainWindow::createGameController(GameParams const& params)
{
  Q_ASSERT(game_model_ != nullptr);
  Q_ASSERT(game_window_ != nullptr);
//  Q_ASSERT(current_player_ < Logic::c_max_player_count);
  game_controller_ = Logic::createGameController(game_window_.data(),
    { game_model_, params.gameSpeed() });
}

void MainWindow::createGameView(GameParams const&)
{
  QQmlEngine* engine = qmlEngine(this);
  QQmlComponent component(engine, QUrl(QStringLiteral("qrc:/GameWindow.qml")));
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
