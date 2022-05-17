#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "math.h"

namespace Rehenz
{
	// require link user32.lib

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

	class Mouse
	{
	private:
		POINT prev, current;
		bool prev_invalid, current_invalid;
		Point2I move;

	public:
		Mouse();
		~Mouse();

		void Present();

		inline Point2I GetMove() { return move; }
		inline int GetMoveX() { return move.x; }
		inline int GetMoveY() { return move.y; }

		void SetToPrev();
	};
}