#include "window_fc.h"
#include <timeapi.h>
#include <utility>

namespace Rehenz
{
	SimpleWindowWithFC::SimpleWindowWithFC(HINSTANCE _hinstance, int _width, int _height, String _title_base)
		: SimpleWindow(_hinstance, _width, _height, _title_base), title_base(std::move(_title_base)), fps_counter(timeGetTime)
	{
		fps_counter.LockFps(100);
		auto updateFps = [this](Rehenz::uint fps)
		{
			this->SetTitle(this->title_base + TEXT(" fps:") + ToString(fps));
		};
		fps_counter.UpdateFpsCallback = updateFps;
	}

	SimpleWindowWithFC::~SimpleWindowWithFC()
	{
	}

	void SimpleWindowWithFC::Present()
	{
		fps_counter.Present();
	}

}