#include <string.h>

extern void command_read(int argc, wchar_t** argv);
extern void command_learn(int argc, wchar_t** argv);

int wmain(int argc, wchar_t** argv)
{
	if (argc < 3)
	{
		return 1;
	}

	if (wcscmp(argv[1], L"read") == 0)
	{
		command_read(argc, argv);
	}
	else if (wcscmp(argv[1], L"learn") == 0)
	{
		command_learn(argc, argv);
	}

	return 0;
}
