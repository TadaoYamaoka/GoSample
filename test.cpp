#include <atltrace.h>
#define TRACE ATLTRACE

#include "test.h"
#include "Board.h"
#include "UCTSample.h"

#ifdef TEST

void test_001() {
	Color test_board[] = {
	//  1  2  3  4  5  6  7  8  9
		2, 2, 2, 2, 2, 1, 0, 1, 1, // 1
		2, 2, 2, 2, 2, 1, 1, 1, 1, // 2 
		2, 2, 2, 2, 2, 1, 1, 0, 1, // 3
		2, 2, 2, 2, 2, 1, 1, 1, 1, // 4
		2, 2, 2, 2, 2, 1, 1, 2, 2, // 5
		1, 1, 1, 1, 2, 1, 1, 2, 2, // 6
		1, 0, 0, 0, 0, 0, 0, 2, 2, // 7
		2, 2, 2, 2, 2, 2, 2, 2, 2, // 8
		2, 2, 2, 2, 2, 2, 2, 2, 2  // 9
	};
	Board board(test_board, 9);

	UCTSample player;
	int xy = player.select_move(board, BLACK);

	for (int i = 0; i < player.root->child_num; i++)
	{
		UCTNode* child = player.root->child + i;
		TRACE(L"x,y = %d,%d : win = %d : playout_num = %d\n", get_x(child->xy), get_y(child->xy), child->win_num, child->playout_num);
	}
	TRACE(L"xy = %d\n", xy);
}

void test_002() {
	Color test_board[] = {
		//  1  2  3  4  5  6  7  8  9
		2, 2, 2, 2, 2, 1, 0, 1, 1, // 1
		2, 2, 2, 2, 2, 1, 1, 1, 1, // 2 
		2, 2, 2, 2, 2, 1, 1, 0, 1, // 3
		2, 2, 2, 2, 2, 1, 1, 1, 1, // 4
		2, 2, 2, 2, 2, 1, 1, 2, 2, // 5
		1, 1, 1, 1, 2, 1, 1, 2, 2, // 6
		1, 0, 0, 0, 0, 0, 0, 2, 2, // 7
		2, 2, 2, 2, 2, 2, 2, 2, 2, // 8
		2, 2, 2, 2, 2, 2, 2, 2, 2  // 9
	};
	Board board(test_board, 9);

	UCTSample player;
	int xy = player.select_move(board, WHITE);

	for (int i = 0; i < player.root->child_num; i++)
	{
		UCTNode* child = player.root->child + i;
		TRACE(L"x,y = %d,%d : win = %d : playout_num = %d\n", get_x(child->xy), get_y(child->xy), child->win_num, child->playout_num);
	}
	TRACE(L"xy = %d\n", xy);
}

void test_003() {
	Color test_board[] = {
		//  1  2  3  4  5  6  7  8  9
		0, 0, 2, 1, 0, 0, 2, 1, 0,
		0, 2, 0, 1, 0, 0, 0, 0, 1,
		2, 0, 0, 1, 0, 2, 2, 1, 0,
		0, 2, 0, 0, 1, 0, 1, 0, 0,
		0, 1, 1, 1, 0, 0, 1, 2, 0,
		0, 0, 0, 0, 2, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 1, 0, 1, 0,
		0, 0, 0, 2, 1, 2, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	Board board(test_board, 9);

	UCTSample player;
	int xy = player.select_move(board, WHITE);

	for (int i = 0; i < player.root->child_num; i++)
	{
		UCTNode* child = player.root->child + i;
		TRACE(L"x,y = %d,%d : win = %d : playout_num = %d\n", get_x(child->xy), get_y(child->xy), child->win_num, child->playout_num);
	}
	TRACE(L"xy = %d\n", xy);
}

int wmain()
{

	//test_001();
	//test_002();
	test_003();

	return 0;
}

#endif