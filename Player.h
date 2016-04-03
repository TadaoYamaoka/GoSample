#pragma once

#include "Board.h"

class Player
{
public:
	virtual int select_move(Board& board, Color color) = 0;
};

