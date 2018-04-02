#ifndef GAMECONTROLLERIMPL_H
#define GAMECONTROLLERIMPL_H

#include "../gamecontroller.h"

namespace Logic {

class TimerController : public GameController
{
  Q_OBJECT

public:
  explicit TimerController(QObject* parent, Params const& params);

public slots:
  void addPattern(PatternTrs pattern_trs) override;

protected:
  void timerEvent(QTimerEvent* event) override;

private:
  void makeStep();

  int const step_timer_id_ = 0;
  GameModelMutablePtr game_model_;
};

} // Logic

#endif // GAMECONTROLLERIMPL_H
