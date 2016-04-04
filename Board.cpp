#include <memory.h>
#include "Board.h"

double KOMI = 6.5;

int BOARD_SIZE;
int BOARD_WIDTH;
int BOARD_MAX;
XY DIR4[4];

bool checked[(19 + 2) * (19 + 2)];

// 呼吸点の数と連結した石の数を取得(内部用再帰処理)
void Board::count_liberties_and_chains_inner(const XY xy, const Color color, int &liberties, int &chains)
{
	// チェック済みする
	checked[xy] = true;

	// 石の数をカウントアップ
	chains++;

	// 4方向について
	for (int d : DIR4)
	{
		XY xyd = xy + d;
		// チェック済みの場合
		if (checked[xyd])
		{
			continue;
		}
		// 空きの場合
		if (board[xyd] == EMPTY) {
			checked[xyd] = true;
			liberties++;
		}
		// 同じ色の場合
		else if (board[xyd] == color) {
			// 再帰呼び出し
			count_liberties_and_chains_inner(xyd, color, liberties, chains);
		}
	}
}

// 呼吸点の数と連結した石の数を取得
void Board::count_liberties_and_chains(const XY xy, const Color color, int &liberties, int &chains)
{
	memset(checked, false, BOARD_MAX);
	liberties = 0;
	chains = 0;

	// 内部用再帰処理
	count_liberties_and_chains_inner(xy, color, liberties, chains);
}

// 石を取る
void Board::capture(const XY xy, const Color color)
{
	board[xy] = EMPTY;
	for (int d : DIR4)
	{
		if (board[xy + d] == color)
		{
			capture(xy + d, color);
		}
	}
}

// 石を打つ
MoveResult Board::move(const XY xy, const Color color, bool fill_eye_err)
{
	// パスの場合
	if (xy == PASS) {
		return SUCCESS;
	}

	int empties = 0; // 空きの数
	int blanks = 0; // 壁の数
	int captures = 0; // 取れる石の数
	int alives = 0; // 生き
	int liberties, chains;
	int tmp_ko;

	// 4方向の
	int around_liberties[] = { 0, 0, 0, 0 }; // 呼吸点
	int around_chains[] = { 0, 0, 0, 0 }; // 連結数

	for (int i = 0; i < 4; i++)
	{
		XY xyd = xy + DIR4[i];
		Color c = board[xyd];
		// 空きの場合
		if (c == EMPTY)
		{
			empties++;
			continue;
		}
		// 壁の場合
		if (c == BLANK)
		{
			blanks++;
			continue;
		}
		// 呼吸点の数と連結した石の数を取得
		count_liberties_and_chains(xyd, c, liberties, chains);

		around_liberties[i] = liberties;
		around_chains[i] = chains;

		// 取ることができる
		if (c == opponent(color) && liberties == 1)
		{
			captures += chains;
			tmp_ko = xyd;
		}
		else if (c == color && liberties >= 2)
		{
			alives++;
		}
	}

	// 自殺手
	if (captures == 0 && empties == 0 && alives == 0)
	{
		return ILLIGAL;
	}
	// コウ
	if (xy == ko)
	{
		return KO;
	}
	// 眼
	if (blanks + alives == 4 && fill_eye_err)
	{
		return EYE;
	}

	// 石を取る
	for (int i = 0; i < 4; i++)
	{
		XY xyd = xy + DIR4[i];
		if (around_liberties[i] == 1 && board[xyd] == opponent(color))
		{
			// 取る
			capture(xyd, opponent(color));
		}
	}

	// 石を打つ
	board[xy] = color;

	// 呼吸点の数と連結した石の数を取得
	count_liberties_and_chains(xy, color, liberties, chains);

	if (captures == 1 && chains == 1 && liberties == 1)
	{
		ko = tmp_ko;
	}
	else {
		ko = 0;
	}

	return SUCCESS;
}