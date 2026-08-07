#pragma once
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

namespace U {
	namespace Console {
		void Init();
		void Free();
		
		// Log levels could be expanded, but sticking to user request "info"
		void Log(const char* fmt, ...);
	}
}
