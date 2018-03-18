#include "gamelogic.h"
#include "../Utilities/rleparser.h"

namespace Logic {

GameField::GameField(Params const& params)
  : cells_(params.cells_)
  , all_patterns_(Utilities::preparePatterns())
{}

} // Logic
