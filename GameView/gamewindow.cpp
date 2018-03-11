#include <QQmlContext>
#include <QQmlEngine>
#include "gamewindow.h"

namespace View {

Logic::SizeT GameWindow::patternCount() const
{
  return game_field_->allPatterns()->patternCount();
}

QVariant GameWindow::patternModel(int idx) const
{
  return QVariant::fromValue(PatternModel(game_field_->allPatterns()->patternAt(idx)));
}

void GameWindow::initialize(Logic::GameField& game_field)
{
  game_view_ = findChild<GameView*>();
  game_field_ = &game_field;
  game_view_->initialize(game_field_->cells());
}

} // View
