#include <iostream>
#include "window.h"
#include "input.h"
#include "dx9.h"

using std::cout;
using std::endl;
using std::cin;

int dx9_example()
{
	SimpleWindow window(GetModuleHandle(nullptr), 800, 600, "dx9");
	if (!window.CheckWindowState())
	{
		cout << "create window - FAILED" << endl;
		return 1;
	}
	else
		cout << "create window - SUCCEEDED" << endl;

	IDirect3DDevice9* device = CreateSimpleDx9Device(&window);
	if (!device)
	{
		cout << "CreateSimpleDx9Device() - FAILED" << endl;
		return 1;
	}
	else
		cout << "CreateSimpleDx9Device() - SUCCEEDED" << endl;

	while (true)
	{
		D3DCOLOR color = 0x00000000; // black
		if (KeyIsDown('1')) color = 0x00000000;
		else if (KeyIsDown('2')) color = 0x00ff0000;
		else if (KeyIsDown('3')) color = 0x0000ff00;
		else if (KeyIsDown('4')) color = 0x000000ff;
		else if (KeyIsDown('5')) color = 0x00ffff00;
		else if (KeyIsDown('6')) color = 0x00ff00ff;
		else if (KeyIsDown('7')) color = 0x0000ffff;
		else if (KeyIsDown('8')) color = 0x00ffffff;
		device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
		device->Present(nullptr, nullptr, nullptr, nullptr);

		SimpleMessageProcess();

		if (!window.CheckWindowState() || KeyIsDown('Q'))
			break;
	}

	if (device)
	{
		device->Release();
		cout << "release device" << endl;
		device = nullptr;
	}

	return 0;
}

int main()
{
	cout << "DirectX learning ..." << endl;
	return dx9_example();
}