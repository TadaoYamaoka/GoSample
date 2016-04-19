#pragma once

#include <memory.h>

typedef signed char Color;
typedef int XY; // boardの座標を表す値

const Color EMPTY = 0;
const Color BLACK = 1;
const Color WHITE = 2;
const Color BLANK = 3;

inline Color opponent(const Color color)
{
	return color ^ 0x3;
}

const int PASS = 0;

extern double KOMI;

extern int BOARD_SIZE;
extern int BOARD_WIDTH;
extern int BOARD_MAX;
extern XY DIR4[4];

inline int get_x(const XY xy)
{
	return xy % BOARD_WIDTH;
}

inline int get_y(const XY xy)
{
	return xy / BOARD_WIDTH;
}

inline XY get_xy(const int x, const int y)
{
	return x + BOARD_WIDTH * y;
}

enum MoveResult { SUCCESS, ILLIGAL, KO, EYE };

// ボード
class Board
{
	Color board[(19+2) * (19+2)];

	// 呼吸点の数と連結した石の数を取得(内部用再帰処理)
	void count_liberties_and_chains_inner(const XY xy, const Color color, int &liberties, int &chains) const;

	// 石を取る
	void capture(const XY xy, const Color color);

public:
	XY ko; // コウ

	Board() {}
	Board(const int size) {
		init(size);
	}
	Board(const Board& src) {
		memcpy(board, src.board, BOARD_MAX);
		ko = src.ko;
	}
	Board(const Color* src, const int size) {
		init(size);
		for (int y = 0; y < size; y++)
		{
			for (int x = 0; x < size; x++)
			{
				board[BOARD_WIDTH * (y + 1) + x + 1] = src[size * y + x];
			}
		}
	}
	~Board() {}


	void init(const int size) {
		BOARD_SIZE = size;
		BOARD_WIDTH = BOARD_SIZE + 2;
		BOARD_MAX = BOARD_WIDTH * BOARD_WIDTH;
		memset(board, EMPTY, BOARD_MAX);
		for (int x = 0; x < BOARD_SIZE + 2; x++)
		{
			board[x] = BLANK;
		}
		for (int y = 1; y < BOARD_SIZE + 1; y++)
		{
			board[BOARD_WIDTH * y + 0] = BLANK;
			board[BOARD_WIDTH * y + (BOARD_WIDTH - 1)] = BLANK;
		}
		for (int x = 0; x < BOARD_SIZE + 2; x++)
		{
			board[BOARD_WIDTH * (BOARD_WIDTH - 1) + x] = BLANK;
		}
		DIR4[0] = -1;
		DIR4[1] = 1;
		DIR4[2] = -BOARD_WIDTH;
		DIR4[3] = BOARD_WIDTH;
		ko = 0;
	}

	const Color operator[](const size_t i) const {
		return board[i];
	}

	Color& operator[](const size_t i) {
		return board[i];
	}

	// 石を打つ
	MoveResult move(const XY xy, const Color color, bool fill_eye_err = true);

	// 呼吸点の数と連結した石の数を取得
	void count_liberties_and_chains(const XY xy, const Color color, int &liberties, int &chains) const;

	// 合法手か
	MoveResult is_legal(const XY xy, const Color color, bool fill_eye_err) const;
};

