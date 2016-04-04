#pragma once
#include "Player.h"
class Human : public Player
{
	XY xy;
public:
	void init() {
		xy = -1;
	}
	virtual XY select_move(Board& board, Color color);
	void set_xy(XY xy);
};
