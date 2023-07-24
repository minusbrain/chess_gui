#include <chess_player.h>

class HumanGuiPlayer : public ChessPlayer {
   public:
    HumanGuiPlayer(const std::string& name);

    Move getMove(const Board& board, const std::vector<Move>& potentialMoves) override;
    bool useGetMove() override { return false; }
};