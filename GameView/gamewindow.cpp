#include <QQmlContext>
#include <QQmlEngine>
#include "gamewindow.h"

namespace View {

Logic::SizeT GameWindow::patternCount() const
{
  return game_field_->allPatterns()->patternCount();
}

QVariant GameWindow::patternModelAt(int idx) const
{
  return QVariant::fromValue(PatternModel(game_field_->allPatterns()->patternAt(idx)));
}

void GameWindow::initialize(Logic::GameField& game_field)
{
  game_view_ = findChild<GameView*>();
  game_field_ = &game_field;
  game_view_->initialize(game_field_->cells());
  connect(game_view_, SIGNAL(patternSelected(PatternTRS)), this, SLOT(addPattern(PatternTRS)));
}

void GameWindow::addPattern(PatternTRS pattern_trs)
{
  auto const& pattern = pattern_trs.first;
  auto const& trs = pattern_trs.second;
  Q_ASSERT(pattern != nullptr);
  for (auto const& point : pattern->points())
  {

  }
}

} // View
