#include "input.h"

namespace Rehenz
{
	Mouse::Mouse() : prev_invalid(true), current_invalid(true), move(0, 0)
	{
		prev.x = prev.y = 0;
		current.x = current.y = 0;
	}

	Mouse::~Mouse()
	{
	}

	void Mouse::Present()
	{
		if (current_invalid)
		{
			if (GetCursorPos(&current) == 0)
				return;
			current_invalid = false;
		}
		else
		{
			prev.x = current.x;
			prev.y = current.y;
			prev_invalid = false;
			if (GetCursorPos(&current) == 0)
			{
				current.x = prev.x;
				current.y = prev.y;
			}
			move.x = current.x - prev.x;
			move.y = current.y - prev.y;
		}
	}

	void Mouse::SetToPrev()
	{
		if (!prev_invalid)
		{
			SetCursorPos(prev.x, prev.y);
			current.x = prev.x;
			current.y = prev.y;
		}
	}

}