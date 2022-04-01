#include <iostream>
#include "Rehenz/window.h"
#include "input.h"
#include <memory>

using std::cout;
using std::endl;
using std::cin;

int main()
{
	cout << "dx11 learning ..." << endl;
	auto window = std::make_shared<Rehenz::SimpleWindowWithFC>(GetModuleHandle(nullptr), 1000, 800, "dx11");
	while (window->CheckWindowState())
	{
		window->Present();
		Rehenz::SimpleMessageProcess();
		if (KeyIsDown('Q'))
			break;
	}
	return 0;
}
