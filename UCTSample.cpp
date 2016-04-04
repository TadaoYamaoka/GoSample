#include <stdlib.h>
#include <math.h>
#include "UCTSample.h"

const int FPU = 10; // First Play Urgency
const double C = 0.5; // UCB定数
const int THR = 20; // ノード展開の閾値
int PLAYOUT_MAX = 1000;

const int NODE_MAX = 30000;
UCTNode node_pool[NODE_MAX]; // ノードプール(高速化のため動的に確保しない)
UCTNode* p_node_pool;

UCTNode* create_root_node()
{
	p_node_pool = node_pool;
	p_node_pool->playout_num_sum = 0;
	p_node_pool->child_num = 0;
	return p_node_pool;
}

UCTNode* create_node(const XY xy)
{
	p_node_pool++;
	p_node_pool->xy = xy;
	p_node_pool->playout_num = 0;
	p_node_pool->playout_num_sum = 0;
	p_node_pool->win_num = 0;
	p_node_pool->child_num = 0;
	return p_node_pool;
}

// ノード展開
void UCTNode::expand_node(const Board& board)
{
	for (XY xy = BOARD_SIZE + 3; xy < BOARD_MAX - (BOARD_SIZE + 3); xy++)
	{
		if (board[xy] == EMPTY)
		{
			create_node(xy);
			child_num++;
		}
	}
	// PASSを追加
	child = create_node(PASS);
	child_num++;

	child -= (child_num - 1); // 先頭のポインタ
}

// 終局 勝敗を返す
int end_game(const Board& board, const Color color)
{
	// 中国ルールで数える
	int score = 0;
	// 石の数
	int stone_num[] = { 0, 0, 0, 0 };

	for (XY xy = BOARD_SIZE + 3; xy < BOARD_MAX - (BOARD_SIZE + 3); xy++)
	{
		Color c = board[xy];
		stone_num[c]++;
		if (c != EMPTY)
		{
			continue;
		}
		int mk[] = { 0, 0, 0, 0 }; // 各色の4方向の石の数
		for (int d : DIR4)
		{
			mk[board[xy + d]]++;
		}
		// 黒の眼
		if (mk[BLACK] > 0 && mk[WHITE] == 0)
		{
			score++;
		}
		// 白の眼
		if (mk[WHITE] > 0 && mk[BLACK] == 0)
		{
			score--;
		}
	}

	score = stone_num[BLACK] - stone_num[WHITE];
	double final_score = score - KOMI;

	if (color == BLACK)
	{
		if (final_score > 0) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if (final_score < 0)
		{
			return 1;
		}
		else {
			return 0;
		}
	}
}

// プレイアウト
int playout(Board& board, UCTNode* node, const Color color, const Color root_color)
{
	int possibles[19 * 19]; // 動的に確保しない

	// 終局までランダムに打つ
	Color color_tmp = color;
	int pre_xy = -1;
	for (int loop = 0; loop < BOARD_MAX + 200; loop++)
	{
		// 候補手一覧
		int possibles_num = 0;
		for (XY xy = BOARD_SIZE + 3; xy < BOARD_MAX - (BOARD_SIZE + 3); xy++)
		{
			if (board[xy] == EMPTY)
			{
				possibles[possibles_num++] = xy;
			}
		}

		int selected;
		while (true)
		{
			if (possibles_num == 0)
			{
				selected = PASS;
			}
			else {
				// ランダムで手を選ぶ
				selected = possibles[rand() % possibles_num];
			}

			// 石を打つ
			MoveResult err = board.move(selected, color_tmp);

			if (err == SUCCESS)
			{
				break;
			}

			// 手を削除
			possibles[selected] = possibles[possibles_num - 1];
			possibles_num--;
		}

		// 連続パスで終了
		if (selected == PASS && pre_xy == PASS)
		{
			break;
		}

		// 一つ前の手を保存
		pre_xy = selected;

		// プレイヤー交代
		color_tmp = opponent(color_tmp);
	}

	// 終局 勝敗を返す
	return end_game(board, root_color);
}

// UCBからプレイアウトする手を選択
UCTNode* select_node_with_ucb(UCTNode* node)
{
	UCTNode* selected_node;
	double max_ucb = -999;
	for (int i = 0; i < node->child_num; i++)
	{
		UCTNode* child = node->child + i;
		double ucb;
		if (child->playout_num == 0)
		{
			// 未実行
			ucb = FPU + double(rand()) * FPU / RAND_MAX;
		}
		else {
			ucb = double(child->win_num) / child->playout_num + C * sqrt(log(node->playout_num_sum) / child->playout_num);
		}

		if (ucb > max_ucb)
		{
			max_ucb = ucb;
			selected_node = child;
		}
	}

	return selected_node;
}

// UCT
int search_uct(Board& board, const Color color, UCTNode* node, Color root_color)
{
	// UCBからプレイアウトする手を選択
	UCTNode* selected_node;
	while (true)
	{
		selected_node = select_node_with_ucb(node);
		MoveResult err = board.move(selected_node->xy, color);
		if (err == SUCCESS)
		{
			break;
		}
		else {
			// 除外
			node->child_num--;
			*selected_node = node->child[node->child_num]; // 値コピー
		}
	}

	int win;

	// 閾値以下の場合プレイアウト
	if (selected_node->playout_num < THR)
	{
		win = playout(board, selected_node, opponent(color), root_color);
	}
	else {
		// ノードを展開
		if (selected_node->child_num == 0)
		{
			selected_node->expand_node(board);
		}
		win = search_uct(board, opponent(color), selected_node, root_color);
	}

	// 勝率を更新
	selected_node->win_num += win;
	selected_node->playout_num++;
	node->playout_num_sum++;

	return win;
}

// 打つ手を選択
XY UCTSample::select_move(Board& board, Color color)
{
	root = create_root_node();
	root->expand_node(board);

	for (int i = 0; i < PLAYOUT_MAX; i++)
	{
		// 局面コピー
		Board board_tmp = board;

		// UCT
		search_uct(board_tmp, color, root, color);
	}

	// 最もプレイアウト数が多い手を選ぶ
	UCTNode* best_move;
	int num_max = -999;
	double rate_min = 1; // 勝率
	double rate_max = 0; // 勝率
	for (int i = 0; i < root->child_num; i++)
	{
		UCTNode* child = root->child + i;
		if (child->playout_num > 0)
		{
			int num = child->playout_num;
			if (num > num_max)
			{
				best_move = child;
				num_max = num;
			}

			double rate;
			if (rate_min == 1)
			{
				rate = double(child->win_num) / child->playout_num;
				if (rate < rate_min)
				{
					rate_min = rate;
				}
			}
			if (rate_max == 0)
			{
				rate = double(child->win_num) / child->playout_num;
				if (rate > rate_max)
				{
					rate_max = rate;
				}
			}
		}
	}

	if (rate_min == 1)
	{
		return PASS;
	}
	if (rate_max == 0)
	{
		return PASS;
	}

	return best_move->xy;
}
