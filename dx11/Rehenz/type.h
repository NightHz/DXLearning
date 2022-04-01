#pragma once
#include <string>

namespace Rehenz
{
	typedef unsigned int uint;
	typedef unsigned char uchar;
	typedef long long llong;
	typedef unsigned long long ullong;

	typedef unsigned char byte;
	typedef unsigned short word;
	typedef unsigned long dword;
	typedef unsigned long long qword;

#ifdef UNICODE
	typedef std::wstring String;
#else
	typedef std::string String;
#endif // UNICODE
}