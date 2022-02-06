#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// letter and number are the same as ASCII code
inline bool KeyIsDown(int vKey)
{
	return (GetAsyncKeyState(vKey) & 0x8000);
}

// letter and number are the same as ASCII code
inline bool KeyIsUp(int vKey)
{
	return !KeyIsDown(vKey);
}
