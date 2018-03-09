#include "gamewindow.h"

namespace View {

void GameWindow::initialize(Logic::GameField& game_field)
{
  game_view_.set(findChild<GameView*>());
  game_field_.set(&game_field);
  game_view_.getMutable().initialize(game_field_.get().cells());
}

} // View
