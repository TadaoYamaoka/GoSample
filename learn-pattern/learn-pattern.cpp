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
	// パターン
	PatternMap patterns;

	// パターン読み込み
	int game_num = 0;
	int position_num = 0;
	for (int i = 2; i < argc; i++)
	{
		// 入力フォルダ
		wchar_t* input_dir = argv[i];
		read_pattern_in_dir(input_dir, patterns, game_num, position_num);
	}

	// 出力
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

	// 上位100
	dump_pattern_top100(patterns);
}

void read_pattern_in_dir(const wchar_t* input_dir, PatternMap &patterns, int &game_num, int &position_num)
{
	// 入力ファイル一覧
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

		// パターン学習
		read_pattern((finddir + L"\\" + win32fd.cFileName).c_str(), patterns, game_num, position_num);
	} while (FindNextFile(hFind, &win32fd));
}

// 対称性を考慮してパターンを検索
PatternMapItr find_pattern(PatternMap &patterns, unsigned int pattern)
{
	PatternMapItr itr;
	itr = patterns.find(pattern);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 90度回転
	unsigned int pattern90 = rotate90(pattern);
	itr = patterns.find(pattern90);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 180度回転
	unsigned int pattern180 = rotate90(pattern90);
	itr = patterns.find(pattern180);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 270度回転
	unsigned int pattern270 = rotate90(pattern180);
	itr = patterns.find(pattern270);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 上下反転
	unsigned int patternref = reflect(pattern);
	itr = patterns.find(patternref);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 90度回転
	unsigned int patternref90 = rotate90(patternref);
	itr = patterns.find(patternref90);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 180度回転
	unsigned int patternref180 = rotate90(patternref90);
	itr = patterns.find(patternref180);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 270度回転
	unsigned int patternref270 = rotate90(patternref180);
	itr = patterns.find(patternref270);
	if (itr != patterns.end())
	{
		return itr;
	}

	return patterns.end();
}

// 対称性を考慮してパターンを検索
ParamMapItr find_pattern(ParamMap &patterns, unsigned int pattern)
{
	ParamMapItr itr;
	itr = patterns.find(pattern);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 90度回転
	unsigned int pattern90 = rotate90(pattern);
	itr = patterns.find(pattern90);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 180度回転
	unsigned int pattern180 = rotate90(pattern90);
	itr = patterns.find(pattern180);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 270度回転
	unsigned int pattern270 = rotate90(pattern180);
	itr = patterns.find(pattern270);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 上下反転
	unsigned int patternref = reflect(pattern);
	itr = patterns.find(patternref);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 90度回転
	unsigned int patternref90 = rotate90(patternref);
	itr = patterns.find(patternref90);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 180度回転
	unsigned int patternref180 = rotate90(patternref90);
	itr = patterns.find(patternref180);
	if (itr != patterns.end())
	{
		return itr;
	}

	// 270度回転
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
	// パターン
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

			// 呼吸点
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

			// 色
			// 黒を基準にする
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
	// 1行目読み飛ばし
	fgets(buf, sizeof(buf), fp);
	// 2行目
	fgets(buf, sizeof(buf), fp);

	// ;で区切る
	char* next = strtok(buf, ";");

	// 指導碁除外
	if (is_sido(next))
	{
		return;
	}

	// 結果取得
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

		// 勝ったプレイヤー
		if (color == win && i >= 20)
		{
			// パターン
			unsigned int pattern = encord_pattern(board, xy, win);

			if (pattern != 0)
			{
				// 対称性を考慮してパターンを検索
				unsigned int patternhit;
				auto itr = find_pattern(patterns, pattern);

				// パターン追加
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

// 90度回転
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

// 上下反転
unsigned int reflect(const unsigned int pattern)
{
	return ((pattern & 0xfff00000) >> (4 * 5)) |
		(pattern & 0x000ff000) |
		((pattern & 0x00000fff) << (4 * 5));
}


void dump_pattern_top100(PatternMap &patterns)
{
	// ソート
	multimap<int, unsigned int> sorted;
	for (auto itr = patterns.begin(); itr != patterns.end(); itr++)
	{
		sorted.insert({ itr->second, itr->first });
	}

	// 上位100
	int i = 0;
	for (auto itr = sorted.rbegin(); itr != sorted.rend() && i < 100; itr++, i++)
	{
		printf("num = %d: ", itr->first);
		dump_pattern(itr->second);
	}
}

void dump_param_top10(ParamMap &patterns, const int n = 10)
{
	// ソート
	multimap<float, unsigned int> sorted;
	for (auto itr = patterns.begin(); itr != patterns.end(); itr++)
	{
		sorted.insert({ itr->second, itr->first });
	}

	// 上位100
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

// 学習
// パターンごとの重みを棋譜に差し手に一致するように調整する
void command_learn(int argc, wchar_t** argv)
{
	// パターンごとの重み
	ParamMap params;

	int game_num;
	int position_num;

	// パターン読み込み
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

	// ソート
	multimap<float, unsigned int> sorted;
	for (auto itr = params.begin(); itr != params.end(); itr++)
	{
		sorted.insert({ itr->second, itr->first });
	}
	// 上位10個のパターンの合計
	int i = 0;
	float sum10 = 0;
	for (auto itr = sorted.rbegin(); itr != sorted.rend() && i < 10; itr++, i++)
	{
		sum10 += itr->first;
	}
	// 上位10個のパターンの平均を1.0となるようにパラメータの初期値を設定
	float rate = 10.0f / sum10;
	for (auto itr = params.begin(); itr != params.end(); itr++)
	{
		itr->second *= rate;
	}

	// 上位10
	dump_param_top10(params);

	game_num = 0;
	position_num = 0;

	// 棋譜を読み込んで学習
	for (int i = 2; i < argc; i++)
	{
		// 入力フォルダ
		wchar_t* input_dir = argv[i];
		learn_pattern_in_dir(input_dir, params, game_num, position_num);
	}

	// 出力
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

	// 上位10
	dump_param_top10(params);

}

void learn_pattern_in_dir(const wchar_t* input_dir, ParamMap &params, int &game_num, int &position_num)
{
	// 入力ファイル一覧
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

		// パターン学習
		learn_pattern((finddir + L"\\" + win32fd.cFileName).c_str(), params, game_num, position_num);
	} while (FindNextFile(hFind, &win32fd));
}

void learn_pattern(const wchar_t* infile, ParamMap &params, int &game_num, int &position_num)
{
	FILE* fp = _wfopen(infile, L"r");
	char buf[10000];
	// 1行目読み飛ばし
	fgets(buf, sizeof(buf), fp);
	// 2行目
	fgets(buf, sizeof(buf), fp);

	// ;で区切る
	char* next = strtok(buf, ";");

	// 指導碁除外
	if (is_sido(next))
	{
		return;
	}

	// 結果取得
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

		// 勝ったプレイヤー
		if (color == win && i >= 20)
		{
			// 教師データのパターン
			unsigned int pattern = encord_pattern(board, xy, win);

			if (pattern != 0)
			{
				float e_sum = 0;
				float e_y = 0;
				float e_etc[19 * 19] = { 0 };

				ParamMap::iterator itr_y = params.end(); // 教師データと一致する手のパラメータ
				ParamMap::iterator itrs[19*19]; // 教師データと一致しない手のパラメータ
				int num = 0;

				// 候補手一覧
				for (XY txy = BOARD_SIZE + 3; txy < BOARD_MAX - (BOARD_SIZE + 3); txy++)
				{
					if (board[txy] == EMPTY && board.is_legal(txy, color, false) == SUCCESS)
					{
						// 候補手パターン
						unsigned int tptn = encord_pattern(board, txy, board[txy]);

						if (tptn != 0)
						{
							// 対称性を考慮してパターンを検索
							auto itr = find_pattern(params, tptn);

							if (itr != params.end())
							{

								// 各手のsoftmaxを計算
								float e = expf(itr->second);
								e_sum += e;

								// 教師データと一致する場合
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
					// 教師データと一致する手のsoftmax
					float y = e_y / e_sum;

					// 教師データと一致する手のパラメータ更新
					itr_y->second -= learning_rate * (y - 1.0f) * itr_y->second;

					// 損失関数
					loss += -logf(y);
					loss_cnt++;
				}

				// 教師データと一致しない手のパラメータ更新
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

	// 損失関数の平均値表示
	printf("loss = %f\n", loss / loss_cnt);

	fclose(fp);
	game_num++;
}