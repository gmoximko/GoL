#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QPointer>

#include "gamemodel.h"

namespace Logic {

class GameController final : public QObject, public Serializable
{
  Q_OBJECT

public:
  struct Params
  {
    GameModelMutablePtr game_model_;
    int update_time_ = 0;
    Score initial_scores_ = 0;
    PlayerId current_player_ = 0;
  };

  explicit GameController(QObject* parent, Params const& params);
  ~GameController() override;

  void onApplicationActive();
  void onApplicationInactive();

public: // Serializable
  SavedData serialize() const override;

public slots:
  void addPattern(PatternTrs pattern_trs);
  bool onStop();
  void onGameSpeedChanged(int update_time);

signals:
  void stepMade(Score scores);

protected:
  void timerEvent(QTimerEvent* event) override;

private:
  using StepId = uint64_t;

  void makeStep();

  int step_timer_id_ = 0;
  PlayerId const player_ = 0;
  Score const score_addition_ = 0;

  GameModelMutablePtr game_model_;
  StepId step_ = 0;
  Score const scores_ = 0;
  int update_time_ = 0;
  bool stopped_ = false;
  bool stopped_due_inactive_ = false;
};
using GameControllerPtr = QPointer<GameController>;

} // Logic

#endif // GAMECONTROLLER_H
