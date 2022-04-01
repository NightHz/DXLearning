#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include "fps_counter.h"

namespace Rehenz
{
	class SimpleWindow
	{
	private:
		const HINSTANCE hinstance;
		HWND hwnd;
		const int width, height;
		String title;

		static bool InitDefaultWindowClass(HINSTANCE hinstance);

	public:
		SimpleWindow(HINSTANCE _hinstance, int _width, int _height, const String& _title);
		SimpleWindow(const SimpleWindow&) = delete;
		SimpleWindow& operator=(const SimpleWindow&) = delete;
		~SimpleWindow();

		HINSTANCE GetHinstance() { return hinstance; }
		HWND GetHwnd() { return hwnd; }
		int GetWidth() { return width; }
		int GetHeight() { return height; }
		String GetTitle() { return title; }

		bool CheckWindowState();
		void SetTitle(const String& _title);
	};

	void SimpleMessageProcess();


	class SimpleWindowWithFC : public SimpleWindow
	{
	private:
	public:
		String title_base;
		FpsCounter fps_counter;

		SimpleWindowWithFC(HINSTANCE _hinstance, int _width, int _height, const String& _title_base);
		SimpleWindowWithFC(const SimpleWindowWithFC&) = delete;
		SimpleWindowWithFC& operator=(const SimpleWindowWithFC&) = delete;
		~SimpleWindowWithFC();

		void Present();
	};
}