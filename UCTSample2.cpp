#include <stdlib.h>
#include <math.h>
#include <map>
#include "UCTSample2.h"

using namespace std;

const int FPU = 10; // First Play Urgency
const double C = 1.0; // UCB定数
const int THR = 20; // ノード展開の閾値

const int NODE_MAX = 100000;
extern UCTNode node_pool[NODE_MAX]; // ノードプール(高速化のため動的に確保しない)
extern UCTNode* p_node_pool;

extern UCTNode* create_root_node();
extern UCTNode* create_node(const XY xy);

// 3*3パターンのパラメータ
typedef map<unsigned int, float> ParamMap;
typedef ParamMap::iterator ParamMapItr;
ParamMap params;

extern unsigned int rotate90(const unsigned int pattern);
extern unsigned int reflect(const unsigned int pattern);
extern unsigned int encord_pattern(const Board &board, const XY xy, const char win);

// パラメータロード
class LoadParams
{
public:
	LoadParams() {
		int game_num;
		int position_num;

		// パターン読み込み
		ParamMap params_tmp;
		FILE* fp = fopen("param33.bin", "rb");
		if (fp == NULL)
		{
			fprintf(stderr, "param file open error.\n");
			return;
		}
		fread(&game_num, sizeof(int), 1, fp);
		fread(&position_num, sizeof(int), 1, fp);
		while (feof(fp) == 0)
		{
			unsigned int ptn;
			fread(&ptn, sizeof(unsigned int), 1, fp);
			float val;
			fread(&val, sizeof(float), 1, fp);

			if (val > 1.0f) // 小さい値は無視
			{
				params_tmp.insert({ ptn, val });
			}
		}
		fclose(fp);

		// 対称性を考慮してパターンを展開
		for (auto itr = params_tmp.begin(); itr != params_tmp.end(); itr++)
		{
			unsigned int pattern = itr->first;
			params.insert({ pattern, itr->second });

			unsigned int pattern90 = rotate90(pattern);
			params.insert({ pattern90, itr->second });

			unsigned int pattern180 = rotate90(pattern90);
			params.insert({ pattern180, itr->second });

			unsigned int pattern270 = rotate90(pattern180);
			params.insert({ pattern270, itr->second });

			// 反転
			unsigned int patternref = reflect(pattern);
			params.insert({ patternref, itr->second });

			unsigned int patternref90 = rotate90(patternref);
			params.insert({ patternref90, itr->second });

			unsigned int patternref180 = rotate90(patternref90);
			params.insert({ patternref180, itr->second });

			unsigned int patternref270 = rotate90(patternref180);
			params.insert({ patternref270, itr->second });
		}
	}
};
LoadParams loadParams;

// 終局 勝敗を返す
extern Color end_game(const Board& board);

// プレイアウト
int UCTSample2::playout(Board& board, UCTNode* node, const Color color)
{
	int possibles[19 * 19]; // 動的に確保しない
	int e[19 * 19];

	// 終局までランダムに打つ
	Color color_tmp = color;
	int pre_xy = node->xy;
	for (int loop = 0; loop < BOARD_MAX + 200; loop++)
	{
		int e_sum = 0;
		int possibles_num = 0;
		int selected_xy;
		int selected_i;

		// 直前の手の周辺でパターンを検索
		if (pre_xy > 0)
		{
			for (int dy = -BOARD_WIDTH; dy <= BOARD_WIDTH; dy += BOARD_WIDTH)
			{
				for (int dx = -1; dx <= 1; dx++)
				{
					if (dy == 0 && dx == 0)
					{
						continue;
					}
					XY xy = pre_xy + dy + dx;
					if (board[xy] == EMPTY)
					{
						possibles[possibles_num] = xy;

						// パターン検索
						unsigned int pattern = encord_pattern(board, xy, color_tmp);
						auto itr = params.find(pattern);

						if (itr != params.end())
						{
							e[possibles_num] = (int)(expf(itr->second) * 1000.0f); // 1000倍して整数にする
							e_sum += e[possibles_num];
						}
						else {
							e[possibles_num] = 0;
						}
						possibles_num++;
					}
				}
			}
		}
		if (e_sum > 0)
		{
			while (true)
			{
				// パターンの確率に応じてランダムに手を選ぶ
				int rnd = e_sum * rand() / RAND_MAX;
				int sum = 0;
				bool found = false;
				for (int i = 0; i < possibles_num; i++)
				{
					sum += e[i];
					if (rnd <= sum)
					{
						selected_i = i;
						found = true;
						break;
					}
				}
				if (!found)
				{
					fprintf(stderr, "pattern not found\n");
				}
				selected_xy = possibles[selected_i];

				// 石を打つ
				MoveResult err = board.move(selected_xy, color_tmp);

				if (err == SUCCESS)
				{
					break;
				}

				// 手を削除
				possibles[selected_i] = possibles[possibles_num - 1];
				e_sum -= e[selected_i];
				e[selected_i] = e[possibles_num - 1];
				possibles_num--;

				if (possibles_num == 0)
				{
					break;
				}
			}
		}


		if (e_sum == 0 || possibles_num == 0)
		{
			// 候補手一覧
			for (XY xy = BOARD_SIZE + 3; xy < BOARD_MAX - (BOARD_SIZE + 3); xy++)
			{
				if (board[xy] == EMPTY)
				{
					possibles[possibles_num] = xy;
					possibles_num++;
				}
			}

			while (true)
			{
				if (possibles_num == 0)
				{
					selected_xy = PASS;
				}
				else {
					// パターンに該当しない場合ランダムに手を選ぶ
					selected_i = rand() % possibles_num;
					selected_xy = possibles[selected_i];
				}

				// 石を打つ
				MoveResult err = board.move(selected_xy, color_tmp);

				if (err == SUCCESS)
				{
					break;
				}

				// 手を削除
				possibles[selected_i] = possibles[possibles_num - 1];
				e_sum -= e[selected_i];
				e[selected_i] = e[possibles_num - 1];
				possibles_num--;
			}
		}

		// 連続パスで終了
		if (selected_xy == PASS && pre_xy == PASS)
		{
			break;
		}

		// 一つ前の手を保存
		pre_xy = selected_xy;

		// プレイヤー交代
		color_tmp = opponent(color_tmp);
	}

	// 終局 勝敗を返す
	Color win = end_game(board);
	if (win == color)
	{
		return 1;
	}
	else {
		return 0;
	}
}

// UCBからプレイアウトする手を選択
UCTNode* UCTSample2::select_node_with_ucb(UCTNode* node)
{
	XY pre_xy = node->xy;
	UCTNode* selected_node;
	double max_ucb = -999;
	for (int i = 0; i < node->child_num; i++)
	{
		UCTNode* child = node->child + i;
		double ucb;
		/*if (child->playout_num < 10 && (child->xy == pre_xy - 1 || child->xy == pre_xy + 1 || child->xy == pre_xy - BOARD_WIDTH || child->xy == pre_xy + BOARD_WIDTH))
		{
			// 直前の手の周辺を優先する
			ucb = 999;

		}
		else */if (child->playout_num == 0)
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
int UCTSample2::search_uct(Board& board, const Color color, UCTNode* node)
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
		win = 1 - playout(board, selected_node, opponent(color));
	}
	else {
		// ノードを展開
		if (selected_node->child_num == 0)
		{
			if (selected_node->expand_node(board))
			{
				win = 1 - search_uct(board, opponent(color), selected_node);
			}
			else {
				// ノードプール不足
				win = 1 - playout(board, selected_node, opponent(color));
			}
		}
		else {
			win = 1 - search_uct(board, opponent(color), selected_node);
		}
	}

	// 勝率を更新
	selected_node->win_num += win;
	selected_node->playout_num++;
	node->playout_num_sum++;

	return win;
}

// 打つ手を選択
XY UCTSample2::select_move(Board& board, Color color)
{
	root = create_root_node();
	root->expand_node(board);

	for (int i = 0; i < PLAYOUT_MAX; i++)
	{
		// 局面コピー
		Board board_tmp = board;

		// UCT
		search_uct(board_tmp, color, root);
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
