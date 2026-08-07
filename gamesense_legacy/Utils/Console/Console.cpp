#include "Console.h"
#include <cstdio>
#include <chrono>
#include <ctime>
#include <vector>

void U::Console::Init()
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	
	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	// Redirect stderr to NUL to suppress engine warning spam (like "Unknown nb_ctl request")
	freopen_s(&fDummy, "NUL", "w", stderr);

	SetConsoleTitleA("majorka.dev");
}

void U::Console::Free()
{
	FreeConsole();
	fclose(stdout);
	fclose(stdin);
	fclose(stderr);
}

void U::Console::Log(const char* fmt, ...)
{
	if (!fmt) return;

	// Time
	SYSTEMTIME t;
	GetLocalTime(&t);

	// Buffer for formatting
	char buffer[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Unified color: Standard Light Gray (looks best for raw console logs)
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	// Example format from screenshot: [00:50:57][15840][info] Message
	printf("[%02d:%02d:%02d][%d][info] %s\n", t.wHour, t.wMinute, t.wSecond, GetCurrentThreadId(), buffer);
}
