#pragma once
#include "Player.h"

extern int PLAYOUT_MAX;

class UCTNode
{
public:
	XY xy;
	int playout_num;
	int playout_num_sum;
	int win_num;
	UCTNode* child; // �q�m�[�h
	int child_num; // �q�m�[�h�̐�

	void expand_node(const Board& board);
};

class UCTSample : public Player
{
public:
	UCTNode* root;
	virtual int select_move(Board& board, Color color);
};
