#include "human_gui_player.h"

#include <cassert>

HumanGuiPlayer::HumanGuiPlayer(const std::string& name) : ChessPlayer(name) {}

Move HumanGuiPlayer::getMove(const Board&, const std::vector<Move>&) { assert(false); }
