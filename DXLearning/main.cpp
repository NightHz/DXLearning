#include <iostream>
#include "window.h"
#include "input.h"
#include "dx9.h"
#include "fps_counter.h"
#include <timeapi.h>
#include <string>

using std::cout;
using std::endl;
using std::cin;

int dx9_example()
{
	HRESULT hr;

	SimpleWindow window(GetModuleHandle(nullptr), 800, 600, "dx9");
	if (!window.CheckWindowState())
		return 1;
	cout << "finish create window" << endl;

	IDirect3DDevice9* device = Dx9::CreateSimpleDx9Device(&window);
	if (!device)
		return 1;
	cout << "finish create dx9 device" << endl;

	Rehenz::FpsCounter fps_counter(timeGetTime);
	auto updateFps = [&window](Rehenz::uint fps) { window.SetTitle((std::string("dx9 fps:") + std::to_string(fps)).c_str()); };
	fps_counter.UpdateFpsCallback = updateFps;
	cout << "start fps_counter" << endl;
	
	// create my cube
	auto obj = Dx9::Object::CreateCube(device);
	if (!obj)
		return 1;
	obj->psi = D3DX_PI * 0.25f;
	obj->theta = D3DX_PI * 0.25f;

	// create d3dx cube
	ID3DXMesh* mesh;
	hr = D3DXCreateTeapot(device, &mesh, nullptr);
	//D3DXCreateBox(device, 2, 2, 2, &mesh, nullptr);
	if (FAILED(hr))
		return 1;

	// create camera
	auto camera = Dx9::Camera::CreateCamera(&window);
	if (!camera)
		return 1;

	// set wireframe
	hr = device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	if (FAILED(hr))
		return 1;

	// set render obj
	std::string active = "obj";

	cout << "finish setup" << endl;

	while (true)
	{
		// clear
		D3DCOLOR color = 0x00ffffff; // white
		if (KeyIsDown('1')) color -= 0x00ff0000; // - red
		if (KeyIsDown('2')) color -= 0x0000ff00; // - green
		if (KeyIsDown('3')) color -= 0x000000ff; // - blue
		device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);

		// fixed pipeline
		// set rendering state
		if (KeyIsDown('4'))
			hr = device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		else if (KeyIsDown('5'))
			hr = device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		if (FAILED(hr))
			return 1;

		// set scene
		if (KeyIsDown('9'))
			active = "obj";
		else if (KeyIsDown('0'))
			active = "mesh";

		// set camera and projection
		if (KeyIsDown('W')) camera->pos.y += 0.1f;
		else if (KeyIsDown('S')) camera->pos.y -= 0.1f;
		if (KeyIsDown('A')) camera->pos.x += 0.1f;
		else if (KeyIsDown('D')) camera->pos.x -= 0.1f;
		if (!camera->Transform(device))
			return 1;

		// set transform
		if (KeyIsDown('I')) obj->theta -= 0.05f;
		else if (KeyIsDown('K')) obj->theta += 0.05f;
		if (KeyIsDown('J')) obj->psi += 0.05f;
		else if (KeyIsDown('L')) obj->psi -= 0.05f;
		if (KeyIsDown('T')) obj->y += 0.1f;
		else if (KeyIsDown('G')) obj->y -= 0.1f;
		if (KeyIsDown('F')) obj->x += 0.1f;
		else if (KeyIsDown('H')) obj->x -= 0.1f;
		if (KeyIsDown('R')) obj->z += 0.1f;
		else if (KeyIsDown('Y')) obj->z -= 0.1f;
		if (!obj->Transform(device))
			return 1;

		// begin
		hr = device->BeginScene();
		if (FAILED(hr))
			return 1;

		// draw
		if (active == "obj")
		{
			if (!obj->Draw(device))
				return 1;
		}
		else if (active == "mesh")
		{
			hr = mesh->DrawSubset(0);
			if (FAILED(hr))
				return 1;
		}
		else
		{
			cout << "wrong active scene" << endl;
			return 1;
		}

		// end
		hr = device->EndScene();
		if (FAILED(hr))
			return 1;

		// present
		device->Present(nullptr, nullptr, nullptr, nullptr);
		fps_counter.Present();

		// msg
		SimpleMessageProcess();
		if (!window.CheckWindowState() || KeyIsDown('Q'))
			break;
	}

	// release
	mesh->Release();
	device->Release();
	cout << "finish release" << endl;

	return 0;
}

int main()
{
	cout << "DirectX learning ..." << endl;
	return dx9_example();
}