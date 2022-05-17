#include "window.h"
#include <utility>

#define WINDOW_CLASS_NAME TEXT("REHENZ")

namespace Rehenz
{
	bool SimpleWindow::InitDefaultWindowClass(HINSTANCE hinstance)
	{
		WNDCLASSEX winclass;
		if (GetClassInfoEx(hinstance, WINDOW_CLASS_NAME, &winclass))
			return true;
		winclass.cbSize = sizeof(WNDCLASSEX);
		winclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		winclass.lpszClassName = WINDOW_CLASS_NAME;
		winclass.lpfnWndProc = DefWindowProc;
		winclass.cbClsExtra = 0;
		winclass.cbWndExtra = 0;
		winclass.hInstance = hinstance;
		winclass.hIcon = nullptr;
		winclass.hIconSm = nullptr;
		winclass.hCursor = nullptr;
		winclass.hbrBackground = nullptr;
		winclass.lpszMenuName = nullptr;
		if (!RegisterClassEx(&winclass))
			return false;
		return true;
	}

	SimpleWindow::SimpleWindow(HINSTANCE _hinstance, int _width, int _height, String _title)
		: hinstance(_hinstance), hwnd(nullptr), width(_width), height(_height)
	{
		SetTitle(std::move(_title));

		if (!InitDefaultWindowClass(hinstance))
		{
			hwnd = nullptr;
			return;
		}

		RECT rc;
		SetRect(&rc, 0, 0, width, height);
		DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
		AdjustWindowRectEx(&rc, style, false, 0);
		hwnd = CreateWindowEx(0, WINDOW_CLASS_NAME, title.c_str(), style,
			0, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hinstance, nullptr);
	}

	SimpleWindow::~SimpleWindow()
	{
		if (hwnd)
		{
			DestroyWindow(hwnd);
			hwnd = nullptr;
		}
	}

	bool SimpleWindow::CheckWindowState()
	{
		if (!hwnd)
			return false;

		HWND hwnd_find = nullptr;
		while (true)
		{
			hwnd_find = FindWindowEx(nullptr, hwnd_find, WINDOW_CLASS_NAME, title.c_str());
			if (!hwnd_find)
			{
				DestroyWindow(hwnd);
				hwnd = nullptr;
				return false;
			}
			else if (hwnd_find == hwnd)
			{
				return true;
			}
			else
				continue;
		}
	}

	void SimpleWindow::SetTitle(String _title)
	{
		title = std::move(_title);
		if (hwnd)
			SetWindowText(hwnd, title.c_str());
	}

	void SimpleMessageProcess()
	{
		MSG msg;
		while (true)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
				break;
		}
	}

}