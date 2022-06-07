#include <iostream>
#include "dx12.h"
#include "Rehenz/input.h"

using std::cout;
using std::wcout;
using std::endl;
using namespace Dx12;
using Rehenz::KeyIsDown;

int main()
{
	cout << "dx12 learning ..." << endl;
	wcout.imbue(std::locale("", LC_CTYPE)); // set to system code

	// print adapter info
	DeviceDx12::PrintAdapterOutputInfo(wcout);

	// create window
	auto window = std::make_shared<Rehenz::SimpleWindowWithFC>(GetModuleHandle(nullptr), 800, 600, "dx12 example");
	if (!window->CheckWindowState())
		return 1;
	window->fps_counter.LockFps(0);
	cout << "finish create window" << endl;

	// create device
	auto device = DeviceDx12::CreateDevice(window.get());
	if (!device)
		return 1;
	cout << "finish create dx12 device" << endl;

	// print device info
	device->PrintSupportInfo(cout);

	// loop
	while (true)
	{
		// present
		device->Present();
		window->Present();
		// msg
		Rehenz::SimpleMessageProcess();
		// exit
		if (KeyIsDown('Q') || !window->CheckWindowState())
			break;
	}

	return 0;
}
