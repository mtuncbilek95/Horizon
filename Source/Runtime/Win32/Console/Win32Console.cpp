#include <Runtime/PAL/Console/Console.h>

#include <Runtime/Log/Terminal.h>

#include <Windows.h>

#include <cstdio>

namespace Horizon::PAL
{
	b8 Console::IsAttached()
	{
		return GetConsoleWindow() != nullptr;
	}

	b8 Console::Hide()
	{
		HWND hConsole = GetConsoleWindow();

		if (!hConsole)
			return false;

		ShowWindow(hConsole, SW_HIDE);
		Terminal::SetConsoleOutput(false);
		return FreeConsole() != 0;
	}

	b8 Console::Show()
	{
		HWND hConsole = GetConsoleWindow();

		if (hConsole)
		{
			ShowWindow(hConsole, SW_SHOW);
			Terminal::SetConsoleOutput(true);
			return true;
		}

		if (!AllocConsole())
			return false;

		FILE* pStream = nullptr;
		freopen_s(&pStream, "CONOUT$", "w", stdout);
		freopen_s(&pStream, "CONOUT$", "w", stderr);
		freopen_s(&pStream, "CONIN$", "r", stdin);

		Terminal::SetConsoleOutput(true);
		return GetConsoleWindow() != nullptr;
	}
}
