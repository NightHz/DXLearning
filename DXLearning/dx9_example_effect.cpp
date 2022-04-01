#include <iostream>
#include "window.h"
#include "input.h"
#include "dx9.h"

namespace Dx9
{
	using std::cout;
	using std::endl;
	using std::cin;

	int dx9_example_effect()
	{
		HRESULT hr = 0;

		SimpleWindowWithFC* window = new SimpleWindowWithFC(GetModuleHandle(nullptr), 800, 600, "dx9");
		if (!window->CheckWindowState())
			return 1;
		cout << "finish create window" << endl;

		IDirect3DDevice9* device = CreateSimpleDx9Device(window);
		if (!device)
			return 1;
		cout << "finish create dx9 device" << endl;

		// create mesh
		auto mesh = Mesh::CreateCubeNormalColorTex1(device);
		if (!mesh)
			return 1;
		auto mesh_teapot = Mesh::CreateD3DXTeapot(device);
		if (!mesh_teapot)
			return 1;

		// create cube
		Object cube(mesh);
		cube.phi = D3DX_PI * 0.25f;
		cube.theta = D3DX_PI * 0.25f;

		// create camera
		Camera camera;
		camera.aspect = static_cast<float>(window->GetWidth()) / window->GetHeight();
		camera.pos.z = 5;

		// create effect
		Effect fx(device, "effect.fx");
		if (!fx)
		{
			cout << "failed : create effect" << endl;
			return 1;
		}
		D3DXHANDLE tech = fx.GetFX()->GetTechniqueByName("Default");

		cout << "finish setup" << endl;

		POINT mouse_pos;
		GetCursorPos(&mouse_pos);
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

			// control camera
			if (KeyIsDown('W')) camera.MoveFront(0.1f);
			else if (KeyIsDown('S')) camera.MoveBack(0.1f);
			if (KeyIsDown('A')) camera.MoveLeft(0.1f);
			else if (KeyIsDown('D')) camera.MoveRight(0.1f);
			if (KeyIsDown(VK_SPACE)) camera.MoveUp(0.1f);
			else if (KeyIsDown(VK_LSHIFT)) camera.MoveDown(0.1f);
			if (KeyIsDown('E')) camera.pos.x = camera.pos.y = 0;
			if (KeyIsDown(VK_MBUTTON))
			{
				POINT mouse_pos2;
				GetCursorPos(&mouse_pos2);
				camera.YawRight(0.003f * (mouse_pos2.x - mouse_pos.x));
				camera.PitchDown(0.003f * (mouse_pos2.y - mouse_pos.y));
				SetCursorPos(mouse_pos.x, mouse_pos.y);
			}
			else
				GetCursorPos(&mouse_pos);

			// set mesh
			if (KeyIsDown('B'))
				cube.mesh = mesh;
			else if (KeyIsDown('N'))
				cube.mesh = mesh_teapot;

			// set effect parameter
			D3DXMATRIX obj_transform = cube.ComputeTransform();
			D3DXMATRIX view_transform = camera.ComputeViewTransform();
			D3DXMATRIX proj_transform = camera.ComputeProjectionTransform();
			D3DXMATRIX to_view_transform = obj_transform * view_transform;
			D3DXMATRIX to_proj_transform = to_view_transform * proj_transform;
			static float t = 0;
			float dt = window->fps_counter.GetLastDeltatime() / 1000.0f;
			t += dt;
			fx.GetFX()->SetMatrix(fx.GetFX()->GetParameterByName(nullptr, "world_transform"), &obj_transform);
			fx.GetFX()->SetMatrix(fx.GetFX()->GetParameterByName(nullptr, "view_transform"), &view_transform);
			fx.GetFX()->SetMatrix(fx.GetFX()->GetParameterByName(nullptr, "proj_transform"), &proj_transform);

			// render
			hr = device->BeginScene();
			if (FAILED(hr))
				return 1;
			if (!fx.RenderObjWithTechnique(device, &cube, tech))
				return 1;
			hr = device->EndScene();
			if (FAILED(hr))
				return 1;

			// present
			hr = device->Present(nullptr, nullptr, nullptr, nullptr);
			if (FAILED(hr))
				return 1;
			window->Present();

			// msg
			SimpleMessageProcess();
			if (!window->CheckWindowState() || KeyIsDown('Q'))
				break;
		}

		// release
		device->Release();
		delete window;
		cout << "finish release" << endl;

		return 0;
	}

}