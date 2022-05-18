#pragma once
#include <utility>

namespace Rehenz
{
	template <typename T>
	inline void Sort(T& a, T& b)
	{
		if (b < a)
			std::swap(a, b);
	}

	template <typename T>
	inline void Sort(T& a, T& b, T& c)
	{
		if (c < b)
			std::swap(b, c);
		if (b < a)
			std::swap(a, b);
		if (c < b)
			std::swap(b, c);
	}
}