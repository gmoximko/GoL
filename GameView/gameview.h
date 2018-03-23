#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#include <QQuickPaintedItem>
#include <QPoint>

namespace View {

class GameWindow;

class GameView : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QPointF pixelsPerCell READ pixelsPerCell CONSTANT)
  Q_PROPERTY(QPoint fieldSize READ fieldSize CONSTANT)

public:
  using QQuickPaintedItem::QQuickPaintedItem;

  QPointF pixelsPerCell() const;
  QPoint fieldSize() const;

  void initialize(GameWindow& game_window);
  void paint(QPainter* painter_ptr) override;

private:
  QPoint cells() const;
  void drawGrid(QPainter& painter) const;
  void drawSelectedCell(QPainter& painter) const;

  GameWindow* game_window_;
};

} // View

#endif // GAMEFIELD_H
