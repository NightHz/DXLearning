#include <iostream>
#include "window.h"
#include "input.h"
#include "dx9.h"
#include <unordered_map>

using std::cout;
using std::endl;
using std::cin;

namespace Dx9
{
	int dx9_example();
	int dx9_example_shader();
	int dx9_example_effect();
}

int main()
{
	cout << "DirectX learning ..." << endl;
	//return Dx9::dx9_example();
	//return Dx9::dx9_example_shader();
	return Dx9::dx9_example_effect();
}