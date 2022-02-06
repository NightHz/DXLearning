#include <iostream>
#include "window.h"
#include "input.h"

using std::cout;
using std::endl;
using std::cin;

int dx9_example()
{
	SimpleWindow window(GetModuleHandle(nullptr), 800, 600, "dx9");

	while (true)
	{
		SimpleMessageProcess();

		if (!window.CheckWindowState() || KeyIsDown('Q'))
			break;
	}

	return 0;
}

int main()
{
	cout << "DirectX learning ..." << endl;
	return dx9_example();
}