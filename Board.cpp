#include <memory.h>
#include "Board.h"

double KOMI = 6.5;

int BOARD_SIZE;
int BOARD_WIDTH;
int BOARD_MAX;
XY DIR4[4];

bool checked[(19 + 2) * (19 + 2)];

// �ċz�_�̐��ƘA�������΂̐����擾(�����p�ċA����)
void Board::count_liberties_and_chains_inner(const XY xy, const Color color, int &liberties, int &chains)
{
	// �`�F�b�N�ς݂���
	checked[xy] = true;

	// �΂̐����J�E���g�A�b�v
	chains++;

	// 4�����ɂ���
	for (int d : DIR4)
	{
		XY xyd = xy + d;
		// �`�F�b�N�ς݂̏ꍇ
		if (checked[xyd])
		{
			continue;
		}
		// �󂫂̏ꍇ
		if (board[xyd] == EMPTY) {
			checked[xyd] = true;
			liberties++;
		}
		// �����F�̏ꍇ
		else if (board[xyd] == color) {
			// �ċA�Ăяo��
			count_liberties_and_chains_inner(xyd, color, liberties, chains);
		}
	}
}

// �ċz�_�̐��ƘA�������΂̐����擾
void Board::count_liberties_and_chains(const XY xy, const Color color, int &liberties, int &chains)
{
	memset(checked, false, BOARD_MAX);
	liberties = 0;
	chains = 0;

	// �����p�ċA����
	count_liberties_and_chains_inner(xy, color, liberties, chains);
}

// �΂����
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

// �΂�ł�
MoveResult Board::move(const XY xy, const Color color, bool fill_eye_err)
{
	// �p�X�̏ꍇ
	if (xy == PASS) {
		return SUCCESS;
	}

	int empties = 0; // �󂫂̐�
	int blanks = 0; // �ǂ̐�
	int captures = 0; // ����΂̐�
	int alives = 0; // ����
	int liberties, chains;
	int tmp_ko;

	// 4������
	int around_liberties[] = { 0, 0, 0, 0 }; // �ċz�_
	int around_chains[] = { 0, 0, 0, 0 }; // �A����

	for (int i = 0; i < 4; i++)
	{
		XY xyd = xy + DIR4[i];
		Color c = board[xyd];
		// �󂫂̏ꍇ
		if (c == EMPTY)
		{
			empties++;
			continue;
		}
		// �ǂ̏ꍇ
		if (c == BLANK)
		{
			blanks++;
			continue;
		}
		// �ċz�_�̐��ƘA�������΂̐����擾
		count_liberties_and_chains(xyd, c, liberties, chains);

		around_liberties[i] = liberties;
		around_chains[i] = chains;

		// ��邱�Ƃ��ł���
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

	// ���E��
	if (captures == 0 && empties == 0 && alives == 0)
	{
		return ILLIGAL;
	}
	// �R�E
	if (xy == ko)
	{
		return KO;
	}
	// ��
	if (blanks + alives == 4 && fill_eye_err)
	{
		return EYE;
	}

	// �΂����
	for (int i = 0; i < 4; i++)
	{
		XY xyd = xy + DIR4[i];
		if (around_liberties[i] == 1 && board[xyd] == opponent(color))
		{
			// ���
			capture(xyd, opponent(color));
		}
	}

	// �΂�ł�
	board[xy] = color;

	// �ċz�_�̐��ƘA�������΂̐����擾
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