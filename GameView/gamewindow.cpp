#include <QQmlContext>
#include <QQmlEngine>

#include "gamewindow.h"

namespace View {

QVariant GameWindow::currentPattern() const
{
  return QVariant::fromValue(PatternModel(current_pattern_));
}

QVariant GameWindow::patternModelAt(int idx) const
{
  return QVariant::fromValue(PatternModel(game_field_->allPatterns()->patternAt(idx)));
}

void GameWindow::initialize(Logic::GameField& game_field)
{
  game_field_ = &game_field;
}

void GameWindow::setCurrentPattern(QVariant const& pattern_model)
{
  auto const model = pattern_model.value<PatternModel>();
  Q_ASSERT(model.pattern() != nullptr);
  if (current_pattern_ != model.pattern())
  {
    current_pattern_ = model.pattern();
    emit currentPatternChanged();
  }
}

void GameWindow::pressed(QPointF point)
{
  selected_cell_ = qMakePair(QPoint(point.x(), point.y()), true);
}

} // View
