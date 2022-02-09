#include <iostream>
#include "window.h"
#include "input.h"
#include "dx9.h"

using std::cout;
using std::endl;
using std::cin;

int dx9_example()
{
	HRESULT hr;

	SimpleWindowWithFC window(GetModuleHandle(nullptr), 800, 600, "dx9");
	if (!window.CheckWindowState())
		return 1;
	cout << "finish create window" << endl;

	IDirect3DDevice9* device = Dx9::CreateSimpleDx9Device(&window);
	if (!device)
		return 1;
	cout << "finish create dx9 device" << endl;
	
	// create mesh
	auto mesh_cube = Dx9::Mesh::CreateCubeColor(device);
	if (!mesh_cube)
		return 1;
	auto mesh_teapot = Dx9::Mesh::CreateD3DXTeapot(device);
	if (!mesh_teapot)
		return 1;

	// create cube
	Dx9::Object cube(mesh_cube);
	cube.psi = D3DX_PI * 0.25f;
	cube.theta = D3DX_PI * 0.25f;

	// create d3dx teapot
	Dx9::Object teapot(mesh_teapot);
	teapot.x = 5;

	// create small cubes
	Dx9::Object cubes_s[100];
	for (int i = 0; i < 100; i++)
	{
		cubes_s[i].mesh = mesh_cube;
		cubes_s[i].sx = 0.2f;
		cubes_s[i].sy = 0.2f;
		cubes_s[i].sz = 0.2f;
		cubes_s[i].y = -5;
		cubes_s[i].x = i / 10 - 4.5f;
		cubes_s[i].z = i % 10 - 4.5f;
	}

	// create camera
	Dx9::Camera camera;
	camera.aspect = static_cast<float>(window.GetWidth()) / window.GetHeight();
	camera.pos.z = 8;

	// create light
	D3DLIGHT9 light;
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Ambient = D3DXCOLOR(0xff, 0, 0, 0xff) * 0.4f;
	light.Diffuse = D3DXCOLOR(0xff, 0, 0, 0xff);
	light.Specular = D3DXCOLOR(0xff, 0, 0, 0xff) * 0.6f;
	light.Direction = D3DXVECTOR3(0.2f, -0.8f, -0.2f);
	hr = device->SetLight(0, &light);
	if (FAILED(hr))
		return 1;
	hr = device->LightEnable(0, true);
	if (FAILED(hr))
		return 1;

	cout << "finish setup" << endl;

	while (true)
	{
		// clear
		int bg_r = 199, bg_g = 220, bg_b = 104;
		if (KeyIsDown('1')) bg_r = 30; // - red
		if (KeyIsDown('2')) bg_g = 30; // - green
		if (KeyIsDown('3')) bg_b = 30; // - blue
		hr = device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(bg_r, bg_g, bg_b), 1.0f, 0);
		if (FAILED(hr))
			return 1;

		// fixed pipeline
		// set rendering state
		if (KeyIsDown('4'))
			hr = device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		else if (KeyIsDown('5'))
			hr = device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID); // default
		if (FAILED(hr))
			return 1;
		if (KeyIsDown('6'))
			hr = device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		else if (KeyIsDown('7'))
			hr = device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD); // default
		if (FAILED(hr))
			return 1;
		if (KeyIsDown('8'))
			hr = device->SetRenderState(D3DRS_LIGHTING, true); // default
		else if (KeyIsDown('9'))
			hr = device->SetRenderState(D3DRS_LIGHTING, false);
		if (FAILED(hr))
			return 1;

		// set camera and projection
		if (KeyIsDown('W')) camera.pos.y += 0.1f;
		else if (KeyIsDown('S')) camera.pos.y -= 0.1f;
		if (KeyIsDown('A')) camera.pos.x += 0.1f;
		else if (KeyIsDown('D')) camera.pos.x -= 0.1f;
		if (KeyIsDown('E')) camera.pos.x = camera.pos.y = 0;
		if (!camera.Transform(device))
			return 1;

		// begin
		hr = device->BeginScene();
		if (FAILED(hr))
			return 1;

		// draw cube
		if (KeyIsDown('I')) cube.theta -= 0.05f;
		else if (KeyIsDown('K')) cube.theta += 0.05f;
		if (KeyIsDown('J')) cube.psi += 0.05f;
		else if (KeyIsDown('L')) cube.psi -= 0.05f;
		if (KeyIsDown('T')) cube.y += 0.1f;
		else if (KeyIsDown('G')) cube.y -= 0.1f;
		if (KeyIsDown('F')) cube.x += 0.1f;
		else if (KeyIsDown('H')) cube.x -= 0.1f;
		if (KeyIsDown('R')) cube.z += 0.1f;
		else if (KeyIsDown('Y')) cube.z -= 0.1f;
		if (!cube.Draw(device))
			return 1;

		// draw teapot
		if (!teapot.Draw(device))
			return 1;

		// draw small cubes
		for (int i = 0; i < 100; i++)
		{
			if (!cubes_s[i].Draw(device))
				return 1;
		}

		// end
		hr = device->EndScene();
		if (FAILED(hr))
			return 1;

		// present
		hr = device->Present(nullptr, nullptr, nullptr, nullptr);
		if (FAILED(hr))
			return 1;
		window.Present();

		// msg
		SimpleMessageProcess();
		if (!window.CheckWindowState() || KeyIsDown('Q'))
			break;
	}

	// release
	device->Release();
	cout << "finish release" << endl;

	return 0;
}

int main()
{
	cout << "DirectX learning ..." << endl;
	return dx9_example();
}