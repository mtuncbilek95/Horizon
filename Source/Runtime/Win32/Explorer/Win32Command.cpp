#include <Runtime/PAL/Explorer/Command.h>

#include <Runtime/Log/Terminal.h>

#include <Windows.h>

namespace Horizon::PAL
{
	void Command::ExecuteInTerminal(const std::string& commandLine)
	{
		if (commandLine.empty())
		{
			Terminal::Warn("Command", "Empty command line");
			return;
		}

		const i32 wideCount = MultiByteToWideChar(CP_UTF8, 0, commandLine.data(), (i32)commandLine.size(), nullptr, 0);

		if (wideCount <= 0)
		{
			Terminal::Error("Command", "Failed to convert command line to UTF-16: {}", commandLine);
			return;
		}

		std::wstring wideCommand(wideCount, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, commandLine.data(), (i32)commandLine.size(), wideCommand.data(), wideCount);

		std::wstring fullCommand = L"cmd.exe /K \"" + wideCommand + L"\"";

		STARTUPINFOW startupInfo = {};
		startupInfo.cb = sizeof(startupInfo);

		PROCESS_INFORMATION processInfo = {};

		const BOOL result = CreateProcessW(nullptr, fullCommand.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo);

		if (!result)
		{
			Terminal::Error("Command", "CreateProcessW failed ({}) for: {}", (u32)GetLastError(), commandLine);
			return;
		}

		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	}
}