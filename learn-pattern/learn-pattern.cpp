#include "..\Board.h"

#include <Windows.h>
#include <string>
#include <map>
#include <set>

using namespace std;


const float learning_rate = 0.01f;

typedef map<unsigned int, int> PatternMap;
typedef PatternMap::iterator PatternMapItr;
typedef map<unsigned int, float> ParamMap;
typedef ParamMap::iterator ParamMapItr;

void command_read(int argc, wchar_t** argv);
void read_pattern_in_dir(const wchar_t* input_dir, PatternMap &patterns, int &game_num, int &position_num);
void read_pattern(const wchar_t* infile, PatternMap &patterns, int &game_num, int &position_num);
unsigned int rotate90(const unsigned int pattern);
unsigned int reflect(const unsigned int pattern);
void dump_pattern_top100(PatternMap &patterns);
void dump_pattern(unsigned int pattern);
void command_learn(int argc, wchar_t** argv);
void learn_pattern_in_dir(const wchar_t* input_dir, ParamMap &patterns, int &game_num, int &position_num);
void learn_pattern(const wchar_t* infile, ParamMap &patterns, int &game_num, int &position_num);

void command_read(int argc, wchar_t** argv)
{
	// �p�^�[��
	PatternMap patterns;

	// �p�^�[���ǂݍ���
	int game_num = 0;
	int position_num = 0;
	for (int i = 2; i < argc; i++)
	{
		// ���̓t�H���_
		wchar_t* input_dir = argv[i];
		read_pattern_in_dir(input_dir, patterns, game_num, position_num);
	}

	// �o��
	FILE* fpo = fopen("pattern33.bin", "wb");
	fwrite(&game_num, sizeof(int), 1, fpo);
	fwrite(&position_num, sizeof(int), 1, fpo);
	for (auto itr = patterns.begin(); itr != patterns.end(); itr++)
	{
		fwrite(&itr->first, sizeof(unsigned int), 1, fpo);
		fwrite(&itr->second, sizeof(int), 1, fpo);
	}
	fclose(fpo);

	printf("read game num = %d\n", game_num);
	printf("position num = %d\n", position_num);
	printf("pattern num = %d\n", patterns.size());

	// ���100
	dump_pattern_top100(patterns);
}

void read_pattern_in_dir(const wchar_t* input_dir, PatternMap &patterns, int &game_num, int &position_num)
{
	// ���̓t�@�C���ꗗ
	wstring finddir(input_dir);
	WIN32_FIND_DATA win32fd;
	HANDLE hFind = FindFirstFile((finddir + L"\\*.sgf").c_str(), &win32fd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "dir open error. %S\n", input_dir);
		return;
	}

	do {
		if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		// �p�^�[���w�K
		read_pattern((finddir + L"\\" + win32fd.cFileName).c_str(), patterns, game_num, position_num);
	} while (FindNextFile(hFind, &win32fd));
}

// �Ώ̐����l�����ăp�^�[��������
PatternMapItr find_pattern(PatternMap &patterns, unsigned int pattern)
{
	PatternMapItr itr;
	itr = patterns.find(pattern);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 90�x��]
	unsigned int pattern90 = rotate90(pattern);
	itr = patterns.find(pattern90);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 180�x��]
	unsigned int pattern180 = rotate90(pattern90);
	itr = patterns.find(pattern180);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 270�x��]
	unsigned int pattern270 = rotate90(pattern180);
	itr = patterns.find(pattern270);
	if (itr != patterns.end())
	{
		return itr;
	}

	// �㉺���]
	unsigned int patternref = reflect(pattern);
	itr = patterns.find(patternref);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 90�x��]
	unsigned int patternref90 = rotate90(patternref);
	itr = patterns.find(patternref90);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 180�x��]
	unsigned int patternref180 = rotate90(patternref90);
	itr = patterns.find(patternref180);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 270�x��]
	unsigned int patternref270 = rotate90(patternref180);
	itr = patterns.find(patternref270);
	if (itr != patterns.end())
	{
		return itr;
	}

	return patterns.end();
}

// �Ώ̐����l�����ăp�^�[��������
ParamMapItr find_pattern(ParamMap &patterns, unsigned int pattern)
{
	ParamMapItr itr;
	itr = patterns.find(pattern);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 90�x��]
	unsigned int pattern90 = rotate90(pattern);
	itr = patterns.find(pattern90);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 180�x��]
	unsigned int pattern180 = rotate90(pattern90);
	itr = patterns.find(pattern180);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 270�x��]
	unsigned int pattern270 = rotate90(pattern180);
	itr = patterns.find(pattern270);
	if (itr != patterns.end())
	{
		return itr;
	}

	// �㉺���]
	unsigned int patternref = reflect(pattern);
	itr = patterns.find(patternref);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 90�x��]
	unsigned int patternref90 = rotate90(patternref);
	itr = patterns.find(patternref90);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 180�x��]
	unsigned int patternref180 = rotate90(patternref90);
	itr = patterns.find(patternref180);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 270�x��]
	unsigned int patternref270 = rotate90(patternref180);
	itr = patterns.find(patternref270);
	if (itr != patterns.end())
	{
		return itr;
	}

	return patterns.end();
}

unsigned int encord_pattern(const Board &board, const XY xy, const char win)
{
	// �p�^�[��
	unsigned int pattern = 0;
	for (int dy = -BOARD_WIDTH; dy <= BOARD_WIDTH; dy += BOARD_WIDTH)
	{
		for (int dx = -1; dx <= 1; dx++)
		{
			if (dx == 0 && dy == 0)
			{
				continue;
			}

			int txy = xy + dx + dy;
			Color tcolor = board[txy];

			// �ċz�_
			int liberties = 0, chains;
			if (tcolor == BLACK || tcolor == WHITE)
			{
				board.count_liberties_and_chains(txy, tcolor, liberties, chains);
			}
			int tliberties = liberties;
			if (liberties >= 3)
			{
				tliberties = 3;
			}

			// �F
			// ������ɂ���
			if (win == 'W')
			{
				switch (tcolor)
				{
				case BLACK:
					tcolor = WHITE;
					break;
				case WHITE:
					tcolor = BLACK;
					break;
				}
			}

			pattern |= (tliberties << 2) + tcolor;

			if (dx != 1 || dy != BOARD_WIDTH)
			{
				pattern <<= 4;
			}
		}
	}

	return pattern;
}

Color get_win_from_re(char* next, const wchar_t* infile)
{
	char* re = strstr(next, "RE[");
	if (re == NULL)
	{
		fprintf(stderr, "RE not found. %S\n", infile);
		return 0;
	}

	char win = re[3];
	if (win == 'b' || win == 'L' || win == -23 || re[15] == -23 || re[12] == -23 || re[6] == -23 || re[19] == -23 || re[13] == -23 || re[11] == -23 || re[9] == -23 || re[4] == -23 || re[9] == 'B' || re[9] == -19 || re[10] == -19 || re[8] == -19 || re[21] == -19 || win == -19 || re[15] == -19 || re[16] == -19)
	{
		win = 'B';
	}
	else if (win == 'w' || win == 'R' || win == -25 || re[15] == -25 || re[12] == -25 || re[6] == -25 || re[19] == -25 || re[13] == -25 || re[11] == -25 || re[9] == -25 || re[4] == -25 || re[9] == 'W' || re[9] == -21 || re[10] == -21 || re[8] == -21 || re[21] == -21 || win == -21 || re[15] == -21 || re[16] == -21)
	{
		win = 'W';
	}
	if (win != 'B' && win != 'W')
	{
		if (win != 'J' && win != 'j' && win != 'V' && win != -27 && win != -26 && win != 'd' && win != '0' && win != '?' && win != -29 && win != -28 && win != ']' && win != 'u' && win != 'U' && win != 's')
		{
			fprintf(stderr, "win illigal. %S\n", infile);
		}
		return 0;
	}

	return (win == 'B') ? BLACK : WHITE;
}

Color get_color_from_sgf(char* next)
{
	char c = next[0];
	Color color;
	if (c == 'B')
	{
		color = BLACK;
	}
	else if (c == 'W') {
		color = WHITE;
	}
	else {
		return 0;
	}
	return color;
}

XY get_xy_from_sgf(char* next)
{
	// PASS
	if (next[2] == ']' || next[2] == '?' || next[1] == ']')
	{
		return PASS;
	}

	int x = next[2] - 'a' + 1;
	int y = next[3] - 'a' + 1;
	XY xy = get_xy(x, y);
	//printf("%s, x, y = %d, %d\n", next, x, y);

	if (next[1] == '\\')
	{
		x = next[3] - 'a' + 1;
		y = next[4] - 'a' + 1;
		xy = get_xy(x, y);
	}
	else if (next[2] == -28)
	{
		xy = get_xy(1, 1);
	}

	return xy;
}

bool is_sido(char* next)
{
	char* ev = strstr(next, "EV[");
	if (ev == NULL)
	{
		return false;
	}
	if (ev[9] == -26 && ev[10] == -116 && ev[11] == -121)
	{
		return true;
	}
	return false;
}

void read_pattern(const wchar_t* infile, PatternMap &patterns, int &game_num, int &position_num)
{
	FILE* fp = _wfopen(infile, L"r");
	char buf[10000];
	// 1�s�ړǂݔ�΂�
	fgets(buf, sizeof(buf), fp);
	// 2�s��
	fgets(buf, sizeof(buf), fp);

	// ;�ŋ�؂�
	char* next = strtok(buf, ";");

	// �w���鏜�O
	if (is_sido(next))
	{
		return;
	}

	// ���ʎ擾
	Color win = get_win_from_re(next, infile);
	if (win == 0)
	{
		fclose(fp);
		return;
	}

	Board board(19);

	int i = 0;
	while ((next = strtok(NULL, ";")) != NULL)
	{
		Color color = get_color_from_sgf(next);
		if (color == 0) {
			continue;
		}

		XY xy = get_xy_from_sgf(next);
		if (xy == PASS)
		{
			continue;
		}

		// �������v���C���[
		if (color == win && i >= 20)
		{
			// �p�^�[��
			unsigned int pattern = encord_pattern(board, xy, win);

			if (pattern != 0)
			{
				// �Ώ̐����l�����ăp�^�[��������
				unsigned int patternhit;
				auto itr = find_pattern(patterns, pattern);

				// �p�^�[���ǉ�
				if (itr == patterns.end())
				{
					patternhit = pattern;
				}
				else {
					patternhit = itr->first;
				}
				patterns[patternhit]++;
				position_num++;
			}
		}

		board.move(xy, color, true);
		i++;
	}

	fclose(fp);
	game_num++;
}

// 90�x��]
unsigned int rotate90(const unsigned int pattern)
{
	return ((pattern & 0xf0000000) >> (4 * 2)) |
		((pattern & 0x0f000000) >> (4 * 3)) |
		((pattern & 0x00f00000) >> (4 * 5)) |
		((pattern & 0x000f0000) << (4 * 2)) |
		((pattern & 0x0000f000) >> (4 * 2)) |
		((pattern & 0x00000f00) << (4 * 5)) |
		((pattern & 0x000000f0) << (4 * 3)) |
		((pattern & 0x0000000f) << (4 * 2));
}

// �㉺���]
unsigned int reflect(const unsigned int pattern)
{
	return ((pattern & 0xfff00000) >> (4 * 5)) |
		(pattern & 0x000ff000) |
		((pattern & 0x00000fff) << (4 * 5));
}


void dump_pattern_top100(PatternMap &patterns)
{
	// �\�[�g
	multimap<int, unsigned int> sorted;
	for (auto itr = patterns.begin(); itr != patterns.end(); itr++)
	{
		sorted.insert({ itr->second, itr->first });
	}

	// ���100
	int i = 0;
	for (auto itr = sorted.rbegin(); itr != sorted.rend() && i < 100; itr++, i++)
	{
		printf("num = %d: ", itr->first);
		dump_pattern(itr->second);
	}
}

void dump_param_top10(ParamMap &patterns, const int n = 10)
{
	// �\�[�g
	multimap<float, unsigned int> sorted;
	for (auto itr = patterns.begin(); itr != patterns.end(); itr++)
	{
		sorted.insert({ itr->second, itr->first });
	}

	// ���100
	int i = 0;
	for (auto itr = sorted.rbegin(); itr != sorted.rend() && i < n; itr++, i++)
	{
		printf("num = %f: ", itr->first);
		dump_pattern(itr->second);
	}
}

void dump_pattern(unsigned int pattern)
{
	for (int i = 7; i >= 0; i--)
	{
		unsigned int point = (pattern >> (4 * i)) & 0xf;
		int liberties = point >> 2;
		int color = point & 0x3;
		printf("%d:%d", liberties, color);
		if (i != 0)
		{
			printf(", ");
		}
		else {
			printf("\n");
		}
	}
}

// �w�K
// �p�^�[�����Ƃ̏d�݂������ɍ�����Ɉ�v����悤�ɒ�������
void command_learn(int argc, wchar_t** argv)
{
	// �p�^�[�����Ƃ̏d��
	ParamMap params;

	int game_num;
	int position_num;

	// �p�^�[���ǂݍ���
	FILE* fp = fopen("pattern33.bin", "rb");
	fread(&game_num, sizeof(int), 1, fp);
	fread(&position_num, sizeof(int), 1, fp);
	while (feof(fp) == 0)
	{
		unsigned int ptn;
		fread(&ptn, sizeof(unsigned int), 1, fp);
		int num;
		fread(&num, sizeof(int), 1, fp);

		params.insert({ ptn, (float)num });
	}
	fclose(fp);

	printf("read game num = %d\n", game_num);
	printf("position num = %d\n", position_num);
	printf("pattern num = %d\n", params.size());

	// �\�[�g
	multimap<float, unsigned int> sorted;
	for (auto itr = params.begin(); itr != params.end(); itr++)
	{
		sorted.insert({ itr->second, itr->first });
	}
	// ���10�̃p�^�[���̍��v
	int i = 0;
	float sum10 = 0;
	for (auto itr = sorted.rbegin(); itr != sorted.rend() && i < 10; itr++, i++)
	{
		sum10 += itr->first;
	}
	// ���10�̃p�^�[���̕��ς�1.0�ƂȂ�悤�Ƀp�����[�^�̏����l��ݒ�
	float rate = 10.0f / sum10;
	for (auto itr = params.begin(); itr != params.end(); itr++)
	{
		itr->second *= rate;
	}

	// ���10
	dump_param_top10(params);

	game_num = 0;
	position_num = 0;

	// ������ǂݍ���Ŋw�K
	for (int i = 2; i < argc; i++)
	{
		// ���̓t�H���_
		wchar_t* input_dir = argv[i];
		learn_pattern_in_dir(input_dir, params, game_num, position_num);
	}

	// �o��
	FILE* fpo = fopen("param33.bin", "wb");
	fwrite(&game_num, sizeof(int), 1, fpo);
	fwrite(&position_num, sizeof(int), 1, fpo);
	for (auto itr = params.begin(); itr != params.end(); itr++)
	{
		fwrite(&itr->first, sizeof(unsigned int), 1, fpo);
		fwrite(&itr->second, sizeof(float), 1, fpo);
	}
	fclose(fpo);

	printf("learned game num = %d\n", game_num);
	printf("learned position num = %d\n", position_num);

	// ���10
	dump_param_top10(params);

}

void learn_pattern_in_dir(const wchar_t* input_dir, ParamMap &params, int &game_num, int &position_num)
{
	// ���̓t�@�C���ꗗ
	wstring finddir(input_dir);
	WIN32_FIND_DATA win32fd;
	HANDLE hFind = FindFirstFile((finddir + L"\\*.sgf").c_str(), &win32fd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "dir open error. %S\n", input_dir);
		return;
	}

	do {
		if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		// �p�^�[���w�K
		learn_pattern((finddir + L"\\" + win32fd.cFileName).c_str(), params, game_num, position_num);
	} while (FindNextFile(hFind, &win32fd));
}

void learn_pattern(const wchar_t* infile, ParamMap &params, int &game_num, int &position_num)
{
	FILE* fp = _wfopen(infile, L"r");
	char buf[10000];
	// 1�s�ړǂݔ�΂�
	fgets(buf, sizeof(buf), fp);
	// 2�s��
	fgets(buf, sizeof(buf), fp);

	// ;�ŋ�؂�
	char* next = strtok(buf, ";");

	// �w���鏜�O
	if (is_sido(next))
	{
		return;
	}

	// ���ʎ擾
	Color win = get_win_from_re(next, infile);
	if (win == 0)
	{
		fclose(fp);
		return;
	}

	Board board(19);

	int i = 0;
	float loss = 0;
	int loss_cnt = 0;
	while ((next = strtok(NULL, ";")) != NULL)
	{
		Color color = get_color_from_sgf(next);
		if (color == 0) {
			continue;
		}

		XY xy = get_xy_from_sgf(next);
		if (xy == PASS)
		{
			continue;
		}

		// �������v���C���[
		if (color == win && i >= 20)
		{
			// ���t�f�[�^�̃p�^�[��
			unsigned int pattern = encord_pattern(board, xy, win);

			if (pattern != 0)
			{
				float e_sum = 0;
				float e_y = 0;
				float e_etc[19 * 19] = { 0 };

				ParamMap::iterator itr_y = params.end(); // ���t�f�[�^�ƈ�v�����̃p�����[�^
				ParamMap::iterator itrs[19*19]; // ���t�f�[�^�ƈ�v���Ȃ���̃p�����[�^
				int num = 0;

				// ����ꗗ
				for (XY txy = BOARD_SIZE + 3; txy < BOARD_MAX - (BOARD_SIZE + 3); txy++)
				{
					if (board[txy] == EMPTY && board.is_legal(txy, color, false) == SUCCESS)
					{
						// ����p�^�[��
						unsigned int tptn = encord_pattern(board, txy, board[txy]);

						if (tptn != 0)
						{
							// �Ώ̐����l�����ăp�^�[��������
							auto itr = find_pattern(params, tptn);

							if (itr != params.end())
							{

								// �e���softmax���v�Z
								float e = expf(itr->second);
								e_sum += e;

								// ���t�f�[�^�ƈ�v����ꍇ
								if (txy == xy)
								{
									e_y = e;
									itr_y = itr;
								}
								else {
									e_etc[num] = e;
									itrs[num] = itr;
									num++;
								}
							}
						}
					}
				}

				if (itr_y != params.end())
				{
					// ���t�f�[�^�ƈ�v������softmax
					float y = e_y / e_sum;

					// ���t�f�[�^�ƈ�v�����̃p�����[�^�X�V
					itr_y->second -= learning_rate * (y - 1.0f) * itr_y->second;

					// �����֐�
					loss += -logf(y);
					loss_cnt++;
				}

				// ���t�f�[�^�ƈ�v���Ȃ���̃p�����[�^�X�V
				for (int i = 0; i < num; i++)
				{
					float y_etc = e_etc[i] / e_sum;
					auto itr = itrs[i];
					itr->second -= learning_rate * y_etc * itr->second;
				}

				position_num++;
			}
		}

		board.move(xy, color, true);
		i++;
	}

	// �����֐��̕��ϒl�\��
	printf("loss = %f\n", loss / loss_cnt);

	fclose(fp);
	game_num++;
}