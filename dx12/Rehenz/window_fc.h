#pragma once
#include "window.h"
#include "fps_counter.h"

namespace Rehenz
{
	// require link winmm.lib
	class SimpleWindowWithFC : public SimpleWindow
	{
	private:
	public:
		String title_base;
		FpsCounter fps_counter;

		SimpleWindowWithFC(HINSTANCE _hinstance, int _width, int _height, String _title_base);
		SimpleWindowWithFC(const SimpleWindowWithFC&) = delete;
		SimpleWindowWithFC& operator=(const SimpleWindowWithFC&) = delete;
		~SimpleWindowWithFC();

		void Present();
	};
}