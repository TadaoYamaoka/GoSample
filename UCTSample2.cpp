#include <stdlib.h>
#include <math.h>
#include <map>
#include "UCTSample2.h"

using namespace std;

const int FPU = 10; // First Play Urgency
const double C = 1.0; // UCB�萔
const int THR = 20; // �m�[�h�W�J��臒l

const int NODE_MAX = 100000;
extern UCTNode node_pool[NODE_MAX]; // �m�[�h�v�[��(�������̂��ߓ��I�Ɋm�ۂ��Ȃ�)
extern UCTNode* p_node_pool;

extern UCTNode* create_root_node();
extern UCTNode* create_node(const XY xy);

// 3*3�p�^�[���̃p�����[�^
typedef map<unsigned int, float> ParamMap;
typedef ParamMap::iterator ParamMapItr;
ParamMap params;

extern unsigned int rotate90(const unsigned int pattern);
extern unsigned int reflect(const unsigned int pattern);
extern unsigned int encord_pattern(const Board &board, const XY xy, const char win);

// �p�����[�^���[�h
class LoadParams
{
public:
	LoadParams() {
		int game_num;
		int position_num;

		// �p�^�[���ǂݍ���
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

			if (val > 1.0f) // �������l�͖���
			{
				params_tmp.insert({ ptn, val });
			}
		}
		fclose(fp);

		// �Ώ̐����l�����ăp�^�[����W�J
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

			// ���]
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

// �I�� ���s��Ԃ�
extern Color end_game(const Board& board);

// �v���C�A�E�g
int UCTSample2::playout(Board& board, UCTNode* node, const Color color)
{
	int possibles[19 * 19]; // ���I�Ɋm�ۂ��Ȃ�
	int e[19 * 19];

	// �I�ǂ܂Ń����_���ɑł�
	Color color_tmp = color;
	int pre_xy = node->xy;
	for (int loop = 0; loop < BOARD_MAX + 200; loop++)
	{
		int e_sum = 0;
		int possibles_num = 0;
		int selected_xy;
		int selected_i;

		// ���O�̎�̎��ӂŃp�^�[��������
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

						// �p�^�[������
						unsigned int pattern = encord_pattern(board, xy, color_tmp);
						auto itr = params.find(pattern);

						if (itr != params.end())
						{
							e[possibles_num] = (int)(expf(itr->second) * 1000.0f); // 1000�{���Đ����ɂ���
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
				// �p�^�[���̊m���ɉ����ă����_���Ɏ��I��
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

				// �΂�ł�
				MoveResult err = board.move(selected_xy, color_tmp);

				if (err == SUCCESS)
				{
					break;
				}

				// ����폜
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
			// ����ꗗ
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
					// �p�^�[���ɊY�����Ȃ��ꍇ�����_���Ɏ��I��
					selected_i = rand() % possibles_num;
					selected_xy = possibles[selected_i];
				}

				// �΂�ł�
				MoveResult err = board.move(selected_xy, color_tmp);

				if (err == SUCCESS)
				{
					break;
				}

				// ����폜
				possibles[selected_i] = possibles[possibles_num - 1];
				e_sum -= e[selected_i];
				e[selected_i] = e[possibles_num - 1];
				possibles_num--;
			}
		}

		// �A���p�X�ŏI��
		if (selected_xy == PASS && pre_xy == PASS)
		{
			break;
		}

		// ��O�̎��ۑ�
		pre_xy = selected_xy;

		// �v���C���[���
		color_tmp = opponent(color_tmp);
	}

	// �I�� ���s��Ԃ�
	Color win = end_game(board);
	if (win == color)
	{
		return 1;
	}
	else {
		return 0;
	}
}

// UCB����v���C�A�E�g������I��
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
			// ���O�̎�̎��ӂ�D�悷��
			ucb = 999;

		}
		else */if (child->playout_num == 0)
		{
			// �����s
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
	// UCB����v���C�A�E�g������I��
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
			// ���O
			node->child_num--;
			*selected_node = node->child[node->child_num]; // �l�R�s�[
		}
	}

	int win;

	// 臒l�ȉ��̏ꍇ�v���C�A�E�g
	if (selected_node->playout_num < THR)
	{
		win = 1 - playout(board, selected_node, opponent(color));
	}
	else {
		// �m�[�h��W�J
		if (selected_node->child_num == 0)
		{
			if (selected_node->expand_node(board))
			{
				win = 1 - search_uct(board, opponent(color), selected_node);
			}
			else {
				// �m�[�h�v�[���s��
				win = 1 - playout(board, selected_node, opponent(color));
			}
		}
		else {
			win = 1 - search_uct(board, opponent(color), selected_node);
		}
	}

	// �������X�V
	selected_node->win_num += win;
	selected_node->playout_num++;
	node->playout_num_sum++;

	return win;
}

// �ł��I��
XY UCTSample2::select_move(Board& board, Color color)
{
	root = create_root_node();
	root->expand_node(board);

	for (int i = 0; i < PLAYOUT_MAX; i++)
	{
		// �ǖʃR�s�[
		Board board_tmp = board;

		// UCT
		search_uct(board_tmp, color, root);
	}

	// �ł��v���C�A�E�g�����������I��
	UCTNode* best_move;
	int num_max = -999;
	double rate_min = 1; // ����
	double rate_max = 0; // ����
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
