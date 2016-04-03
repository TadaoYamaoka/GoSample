#include <windows.h>
#include <stdio.h>
#include <CommCtrl.h>
#include <typeinfo>
#include <time.h>
#include "UCTSample.h"
#include "test.h"

using namespace std;

// プレイヤー一覧
Player* playerList[] = {new UCTSample()};

static bool isPalying = false;
static Board board;
static Player* players[2] = { playerList[0], playerList[0] };
UCTNode result[19*19];
int result_num;

const int MARGIN = 24;
int GRID_SIZE = 9;
const int GRID_WIDTH = 45;
const int INFO_WIDTH = 120;
const int CTRL_HEIGHT = 24;
const int CTRL_MARGIN = 8;

HWND hMainWnd;
DWORD style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;

HINSTANCE hInstance;
HBRUSH hBrushBoard;
HPEN hPenBoard;
HFONT hFontPlayout;

float scaleX, scaleY;

inline int scaledX(const float x) {
	return (int)(x * scaleX);
}
inline int scaledY(const float y) {
	return (int)(y * scaleY);
}

// GTP用
bool isGTPMode = false;
bool gtp_clear_board = false;
int gtp_boardsize = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadProc(LPVOID lpParameter);

#ifndef TEST
int wmain(int argc, wchar_t* argv[]) {
	::hInstance = GetModuleHandle(NULL);
	srand(time(NULL));

	// オプション
	for (int i = 0; i < argc; i++)
	{
		// -gtpでGTPモード
		if (wcscmp(argv[i], L"-gtp") == 0)
		{
			isGTPMode = true;

			setbuf(stdout, NULL);
			setbuf(stderr, NULL);  // stderrに書くとGoGuiに表示される。

			// 監視スレッド起動
			CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
		}
		else {
			// プレイアウト数
			int n = _wtoi(argv[i]);
			if (n > 0)
			{
				PLAYOUT_MAX = n;
			}
		}
	}

	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszMenuName = NULL;
	wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	wcex.lpszClassName = L"GoSample";

	RegisterClassEx(&wcex);

	HDC screen = GetDC(0);
	scaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
	scaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
	ReleaseDC(0, screen);

	RECT rc = { 0, 0, scaledX(MARGIN + GRID_WIDTH * (GRID_SIZE + 1) + MARGIN + INFO_WIDTH + MARGIN), scaledY(MARGIN + GRID_WIDTH * (GRID_SIZE + 1) + MARGIN) };
	AdjustWindowRect(&rc, style, NULL);

	hMainWnd = CreateWindow(
		L"GoSample",
		L"GoSample",
		style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, hInstance, NULL);
	if (hMainWnd == NULL)
	{
		return 0;
	}

	// GDIオブジェクト作成
	hBrushBoard = CreateSolidBrush(RGB(190, 160, 60));
	hPenBoard = (HPEN)CreatePen(PS_SOLID, scaledX(2), RGB(0, 0, 0));
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT lf;
	GetObject(hFont, sizeof(lf), &lf);
	lf.lfHeight = -scaledY(12);
	lf.lfWidth = 0;
	hFontPlayout = CreateFontIndirect(&lf);

	ShowWindow(hMainWnd, SW_SHOW);
	UpdateWindow(hMainWnd);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeleteObject(hBrushBoard);
	DeleteObject(hPenBoard);
	DeleteObject(hFontPlayout);

	return 0;
}
#endif

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND staticPlayers[2];
	static HWND cmbPlayers[2];
	static HWND btnStart;
	const int BTN_ID_START = 0;

	switch (uMsg)
	{
	case WM_CREATE:
	{
		int infoX = scaledX(MARGIN + GRID_WIDTH * (GRID_SIZE + 1) + MARGIN);
		int infoY = scaledY(MARGIN);
		// プレイヤー選択ボックス
		staticPlayers[0] = CreateWindow(WC_STATIC, L"Black:", WS_CHILD | WS_VISIBLE, infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT), hWnd, NULL, hInstance, NULL);
		infoY += scaledY(CTRL_HEIGHT);

		cmbPlayers[0] = CreateWindow(WC_COMBOBOX, L"Player1", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT * 10),
			hWnd, NULL, hInstance, NULL);
		infoY += scaledY(CTRL_HEIGHT + CTRL_MARGIN * 2);

		staticPlayers[1] = CreateWindow(WC_STATIC, L"White:", WS_CHILD | WS_VISIBLE, infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT), hWnd, NULL, hInstance, NULL);
		infoY += scaledY(CTRL_HEIGHT);

		cmbPlayers[1] = CreateWindow(WC_COMBOBOX, L"Player2", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT * 10),
			hWnd, NULL, hInstance, NULL);
		infoY += scaledY(CTRL_HEIGHT + CTRL_MARGIN * 2);

		for (int i = 0; i < 2; i++)
		{
			for (Player* player : playerList)
			{
				const char* name = typeid(*player).name();
				SendMessageA(cmbPlayers[i], CB_ADDSTRING, NULL, (LPARAM)name + 6);
			}
			SendMessage(cmbPlayers[i], CB_SETCURSEL, 0, NULL);
		}

		// スタートボタン
		btnStart = CreateWindow(WC_BUTTON, L"Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT),
			hWnd, BTN_ID_START, hInstance, NULL);

		if (isGTPMode)
		{
			EnableWindow(btnStart, FALSE);
		}

		return 0;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {
		case BTN_ID_START:
		{
			// ボード初期化
			board.init(GRID_SIZE);

			// プレイヤー取得
			for (int i = 0; i < 2; i++)
			{
				int index = SendMessage(cmbPlayers[i], CB_GETCURSEL, NULL, NULL);
				players[i] = playerList[index];
			}

			// 開始
			Color color = BLACK;
			Color pre_xy = -1;

			isPalying = true;
			while (isPalying)
			{
				// 局面コピー
				Board board_tmp = board;

				// 手を選択
				Player* current_player = players[color - 1];
				int xy = current_player->select_move(board_tmp, color);

				// 石を打つ
				MoveResult err = board.move(xy, color);

				if (err != SUCCESS)
				{
					break;
				}

				if (xy == PASS && pre_xy == PASS)
				{
					// 終局
					break;
				}

				pre_xy = xy;
				color = opponent(color);

				// 描画更新
				if (typeid(*current_player) == typeid(UCTSample))
				{
					UCTNode* root = ((UCTSample*)current_player)->root;
					for (int i = 0; i < root->child_num; i++)
					{
						result[i] = root->child[i]; // 値コピー
					}
					result_num = root->child_num;
				}
				InvalidateRect(hWnd, NULL, FALSE);

				// メッセージ処理
				MSG msg;
				while (PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE))
				{
					if (GetMessage(&msg, NULL, 0, 0))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}
			isPalying = false;
			return 0;
		}
		}
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);

		HBRUSH hPrevBrush = (HBRUSH)SelectObject(hDC, hBrushBoard);

		Rectangle(hDC, scaledX(MARGIN), scaledY(MARGIN), scaledX(MARGIN + GRID_WIDTH * (GRID_SIZE + 1)), scaledY(MARGIN + GRID_WIDTH * (GRID_SIZE + 1)));

		HPEN hPrevPen = (HPEN)SelectObject(hDC, hPenBoard);

		for (int x = 1; x < GRID_SIZE + 1; x++)
		{
			int drawX = scaledX(MARGIN + GRID_WIDTH * x);
			MoveToEx(hDC, drawX, scaledY(MARGIN + GRID_WIDTH), NULL);
			LineTo(hDC, drawX, scaledY(MARGIN + GRID_WIDTH * GRID_SIZE));
		}
		for (int y = 1; y < GRID_SIZE + 1; y++)
		{
			int drawY = scaledY(MARGIN + GRID_WIDTH * y);
			MoveToEx(hDC, scaledY(MARGIN + GRID_WIDTH), drawY, NULL);
			LineTo(hDC, scaledY(MARGIN + GRID_WIDTH * GRID_SIZE), drawY);
		}

		// 石を描画
		for (int xy = BOARD_SIZE + 3; xy < BOARD_MAX - (BOARD_SIZE + 3); xy++)
		{
			int x = scaledX(MARGIN + GRID_WIDTH * (get_x(xy) - 0.5f));
			int y = scaledY(MARGIN + GRID_WIDTH * (get_y(xy) - 0.5f));
			if (board[xy] == BLACK)
			{
				SelectObject(hDC, (HBRUSH)GetStockObject(BLACK_BRUSH));
				Ellipse(hDC, x, y, x + GRID_WIDTH, y + GRID_WIDTH);
			}
			else if (board[xy] == WHITE)
			{
				SelectObject(hDC, (HBRUSH)GetStockObject(WHITE_BRUSH));
				Ellipse(hDC, x, y, x + GRID_WIDTH, y + GRID_WIDTH);
			}
		}

		// プレイアウト結果を表示
		if (result_num > 0)
		{
			HFONT hPrevFont = (HFONT)SelectObject(hDC, hFontPlayout);
			SetTextColor(hDC, RGB(255, 0, 0));
			SetBkMode(hDC, TRANSPARENT);
			for (int i = 0; i < result_num; i++)
			{
				int x = get_x(result[i].xy);
				int y = get_y(result[i].xy);
				int drawX = scaledX(MARGIN + GRID_WIDTH * (x - 0.5f));
				int drawY = scaledY(MARGIN + GRID_WIDTH * (y - 0.25f));
				if (x == 0)
				{
					drawX = scaledX(MARGIN);
					drawY = scaledX(MARGIN);
				}
				wchar_t str[20];
				int len = wsprintf(str, L"%3d/%d\n.%d", result[i].win_num, result[i].playout_num, 100 * result[i].win_num / result[i].playout_num);
				RECT rc = { drawX, drawY, drawX + scaledX(GRID_WIDTH), drawY + scaledY(GRID_WIDTH) };
				DrawText(hDC, str, len, &rc, DT_CENTER);
			}
			SelectObject(hDC, hPrevFont);
		}

		SelectObject(hDC, hPrevBrush);
		SelectObject(hDC, hPrevPen);
		EndPaint(hWnd, &ps);

		return 0;
	}
	case WM_SIZE:
	{
		int infoX = scaledX(MARGIN + GRID_WIDTH * (GRID_SIZE + 1) + MARGIN);
		int infoY = scaledY(MARGIN);
		// プレイヤー選択ボックス
		MoveWindow(staticPlayers[0], infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT), TRUE);
		infoY += scaledY(CTRL_HEIGHT);

		MoveWindow(cmbPlayers[0], infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT * 10), TRUE);
		infoY += scaledY(CTRL_HEIGHT + CTRL_MARGIN * 2);

		MoveWindow(staticPlayers[1], infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT), TRUE);
		infoY += scaledY(CTRL_HEIGHT);

		MoveWindow(cmbPlayers[1], infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT * 10), TRUE);
		infoY += scaledY(CTRL_HEIGHT + CTRL_MARGIN * 2);

		// スタートボタン
		MoveWindow(btnStart, infoX, infoY, scaledX(INFO_WIDTH), scaledY(CTRL_HEIGHT), TRUE);

		return 0;
	}
	case WM_DESTROY:
	{
		isPalying = false;
		PostQuitMessage(0);
		return 0;
	}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	const char *gtp_commands[] = {
		"protocol_version",
		"name",
		"version",
		"known_command",
		"list_commands",
		"quit",
		"boardsize",
		"clear_board",
		"komi",
		"play",
		"genmove"
	};

	const char gtp_axis_x[] = { '\0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'k', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T' };

	char line[256];
	char* err;
	//FILE* fp = fopen("out.txt", "a");
	while ((err = gets_s(line, sizeof(line))) != nullptr)
	{
		//fprintf(fp, "%s\n", line);
		//fflush(fp);
		if (strcmp(line, "name") == 0)
		{
			printf("= GoSample\n\n");
		}
		else if (strcmp(line, "protocol_version") == 0)
		{
			printf("= 2\n\n");
		}
		else if (strcmp(line, "version") == 0)
		{
			printf("= 1.0\n\n");
		}
		else if (strcmp(line, "list_commands") == 0)
		{
			printf("= ");
			for (const char* command : gtp_commands)
			{
				printf("%s\n", command);
			}
			printf("\n");
		}
		else if (strncmp(line, "boardsize", 9) == 0)
		{
			GRID_SIZE = atoi(line + 10);
			// ボード初期化
			board.init(GRID_SIZE);

			RECT rc = { 0, 0, scaledX(MARGIN + GRID_WIDTH * (GRID_SIZE + 1) + MARGIN + INFO_WIDTH + MARGIN), scaledY(MARGIN + GRID_WIDTH * (GRID_SIZE + 1) + MARGIN) };
			AdjustWindowRect(&rc, style, NULL);
			SetWindowPos(hMainWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

			printf("= \n\n");
		}
		else if (strcmp(line, "clear_board") == 0)
		{
			board.init(GRID_SIZE);
			printf("= \n\n");
		}
		else if (strncmp(line, "komi", 4) == 0)
		{
			KOMI = atof(line + 5);
			printf("= \n\n");
		}
		else if (strncmp(line, "play", 4) == 0)
		{
			char charColor = line[5];
			Color color = (charColor == 'B') ? BLACK : WHITE;

			if (strcmp(line + 7, "PASS") != 0)
			{
				char charX = line[7];
				int x;
				for (x = 1; x <= 19; x++)
				{
					if (charX == gtp_axis_x[x])
					{
						break;
					}
				}
				int y = GRID_SIZE - atoi(line + 8) + 1;

				int xy = x + BOARD_WIDTH * y;

				board.move(xy, color);

				InvalidateRect(hMainWnd, NULL, FALSE);
			}

			printf("= \n\n");
		}
		else if (strncmp(line, "genmove", 7) == 0)
		{
			char charColor = line[8];
			Color color = (charColor == 'b') ? BLACK : WHITE;

			Player* current_player = players[color - 1];

			int xy = current_player->select_move(board, color);
			board.move(xy, color);

			if (xy == PASS)
			{
				printf("= pass\n\n");
			}
			else {
				printf("= %c%d\n\n", gtp_axis_x[get_x(xy)], GRID_SIZE - get_y(xy) + 1);
			}

			if (typeid(*current_player) == typeid(UCTSample))
			{
				UCTNode* root = ((UCTSample*)current_player)->root;
				for (int i = 0; i < root->child_num; i++)
				{
					result[i] = root->child[i]; // 値コピー
				}
				result_num = root->child_num;
			}
			InvalidateRect(hMainWnd, NULL, FALSE);
		}
		else if (strncmp(line, "final_score", 11) == 0)
		{
			printf("? cannot score\n\n");
		}
		else if (strncmp(line, "quit", 4) == 0)
		{
			printf("= \n\n");
			break;
		}
	}
	//fclose(fp);

	return 0;
}