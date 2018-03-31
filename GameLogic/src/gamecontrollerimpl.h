#ifndef GAMECONTROLLERIMPL_H
#define GAMECONTROLLERIMPL_H

#include <QTimer>

#include "../gamecontroller.h"

namespace Logic {

class GameControllerImpl : public GameController
{
  Q_OBJECT

public:
  explicit GameControllerImpl(QObject* parent, Params const& params);

public slots:
  void addPattern(PatternTrs pattern_trs) override;
  void makeStep();

private:
  GameModelMutablePtr game_model_;
  QTimer timer_;
};

} // Logic

#endif // GAMECONTROLLERIMPL_H
