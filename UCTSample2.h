#pragma once
#include "UCTSample.h"

class UCTSample2 : public UCTSample
{
	int playout(Board& board, UCTNode* node, const Color color);
	int search_uct(Board& board, const Color color, UCTNode* node);
	UCTNode* select_node_with_ucb(UCTNode* node);
public:
	virtual XY select_move(Board& board, Color color);
};

