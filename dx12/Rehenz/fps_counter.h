#pragma once
#include "type.h"
#include <functional>

namespace Rehenz
{
	class FpsCounter
	{
	private:
		using TimeType = unsigned long;

		// save frames per 0.5s
		uint fps[3];
		TimeType fps_t0;
		TimeType t0, t1;
		TimeType lock_time;
		
		std::function<TimeType(void)> GetTime;

	public:
		// update per 0.5s
		std::function<void(uint fps)> UpdateFpsCallback;

		// default lock 60 fps
		FpsCounter(std::function<TimeType(void)> GetTimeFunc);
		~FpsCounter();

		void Present();
		uint GetLastFps();
		// ms unit
		uint GetLastDeltatime();
		// input 0 to unlock fps
		void LockFps(uint fps);
	};
}