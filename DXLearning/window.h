#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class SimpleWindow
{
private:
	const HINSTANCE hinstance;
	HWND hwnd;
	const int width, height;
	char title[80]; // not support Unicode
	
	static bool InitDefaultWindowClass(HINSTANCE hinstance);

public:
	SimpleWindow(HINSTANCE _hinstance, int _width, int _height, const char* _title);
	SimpleWindow(const SimpleWindow&) = delete;
	SimpleWindow& operator=(const SimpleWindow&) = delete;
	~SimpleWindow();

	HINSTANCE GetHinstance() { return hinstance; }
	HWND GetHwnd() { return hwnd; }
	int GetWidth() { return width; }
	int GetHeight() { return height; }
	const char* GetTitle() { return title; }

	bool CheckWindowState();
	void SetTitle(const char* _title);
};

void SimpleMessageProcess();
