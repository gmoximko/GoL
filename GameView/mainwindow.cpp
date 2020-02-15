#include <QQmlEngine>
#include <QQmlContext>
#include <QScopedPointer>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QDir>

#include "../Utilities/qtutilities.h"
#include "mainwindow.h"

namespace View {

namespace {

bool suppressSignals(QObject* obj, bool block)
{
  return obj != nullptr && obj->blockSignals(block);
}

} // namespace

void GameParams::setLobby(Network::LobbyPtr lobby)
{
  lobby_ = std::move(lobby);
  auto const connection = connect(lobby_.data(), &Network::Lobby::ready, this, &GameParams::ready, Qt::UniqueConnection);
  Q_ASSERT(lobby_->lobbyParams() == game_params_);
  Q_ASSERT(connection);
}

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
  loadGame();
}

QQuickItem* MainWindow::createGame(GameParams* game_params)
{
  Q_ASSERT(game_params != nullptr);
  Q_ASSERT(game_model_ == nullptr);
  Q_ASSERT(game_view_ == nullptr);
  Q_ASSERT(game_controller_ == nullptr);

  createGameModel(*game_params);
  createGameView(*game_params);
  createGameController(*game_params);
  initializeLobby(game_params->lobby());

  connect(game_view_.data(), &GameView::patternSelected,
          game_controller_.data(), &Logic::GameController::addPattern);
  connect(game_view_.data(), &GameView::stop,
          game_controller_.data(), &Logic::GameController::onStop);
  connect(game_controller_.data(), &Logic::GameController::stepMade,
          game_view_.data(), &GameView::onStepMade);
  connect(game_view_.data(), &GameView::gameSpeedChanged,
          game_controller_.data(), &Logic::GameController::onGameSpeedChanged);
  suppressSignals(game_network_.data(), true);
  return game_window_.data();
}

bool MainWindow::destroyGame()
{
  auto const result = game_window_ != nullptr;
  game_model_ = nullptr;
  game_window_->deleteLater();
  game_window_.take();
  suppressSignals(game_network_.data(), false);
  return result;
}

void MainWindow::createLobby(GameParams* game_params)
{
  connect(game_network_.data(), &Network::GameNetwork::lobbyCreated,
          game_params, &GameParams::setLobby, Qt::UniqueConnection);
  game_network_->createLobby(game_params->params());
}

void MainWindow::joinLobby(GameParams* game_params)
{
  connect(game_network_.data(), &Network::GameNetwork::lobbyCreated,
          game_params, &GameParams::setLobby, Qt::UniqueConnection);
  game_network_->joinLobby(game_params->params());
}

void MainWindow::aboutToQuit()
{
  qDebug() << "aboutToQuit";
  saveGame();
}

void MainWindow::applicationStateChanged(Qt::ApplicationState state)
{
  qDebug() << "applicationStateChanged " << state;
  switch (state)
  {
  case Qt::ApplicationState::ApplicationSuspended:
    saveGame();
    break;
  case Qt::ApplicationState::ApplicationHidden:
    break;
  case Qt::ApplicationState::ApplicationInactive:
    if (game_controller_ != nullptr)
    {
      game_controller_->onApplicationInactive();
    }
    break;
  case Qt::ApplicationState::ApplicationActive:
    if (game_controller_ != nullptr)
    {
      game_controller_->onApplicationActive();
    }
    break;
  }
}

void MainWindow::saveGame() const
{
  if (game_model_ == nullptr || game_controller_ == nullptr || game_view_ == nullptr)
  {
    return;
  }
  QMap<QString, QVariant> data;
  data["gameModel"] = game_model_->serialize();
  data["gameController"] = game_controller_->serialize();
  data["gameView"] = game_view_->serialize();
  Q_ASSERT(std::none_of(data.cbegin(), data.cend(), [](auto const& piece)
  {
    return piece.toMap().empty();
  }));
  writeSave(data);
}

void MainWindow::loadGame()
{
  auto const save = readSave();
  auto const data = save.toMap();
  if (std::any_of(data.cbegin(), data.cend(), [](auto const& piece)
    {
      return piece.toMap().empty();
    }))
  {
    return;
  }
}

void MainWindow::writeSave(QVariant const &data) const
{
  auto const app_data = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  Q_ASSERT(!app_data.isEmpty());
  QDir().mkpath(app_data);
  QFile saved_game(app_data + "/savedGame");
  if (!saved_game.open(QIODevice::WriteOnly))
  {
    Q_UNREACHABLE();
    return;
  }
  QDataStream stream(&saved_game);
  stream << data;
}

QVariant MainWindow::readSave() const
{
  auto const app_data = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  Q_ASSERT(!app_data.isEmpty());
  QVariant result;
  QFile saved_game(app_data + "/savedGame");
  if (saved_game.open(QIODevice::ReadOnly))
  {
    QDataStream stream(&saved_game);
    stream >> result;
  }
  saved_game.remove();
  return result;
}

void MainWindow::createGameModel(GameParams const& params)
{
  game_model_ = Logic::createGameModel({ params.fieldSize() });
}

void MainWindow::createGameController(GameParams const& params)
{
  Q_ASSERT(game_model_ != nullptr);
  Q_ASSERT(game_window_ != nullptr);
  game_controller_ = new Logic::GameController(game_window_.data(),
    { game_model_
    , params.gameSpeed()
    , params.initialScores()
    , params.currentPlayer()
    });
}

void MainWindow::createGameView(GameParams const&)
{
  QQmlComponent component(qmlEngine(this), QUrl(QStringLiteral("qrc:/GameWindow.qml")));
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

void MainWindow::initializeLobby(Network::LobbyPtr lobby)
{
  Q_ASSERT(game_model_ != nullptr);
  if (lobby != nullptr)
  {
    lobby->initialize(game_model_);
  }
}

} // View
