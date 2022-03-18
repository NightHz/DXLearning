#include <iostream>
#include "window.h"
#include "input.h"
#include "dx9.h"
#include <unordered_map>

namespace Dx9
{
	using std::cout;
	using std::endl;
	using std::cin;

	int dx9_example_shader()
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

		// create texture
		auto tex = Texture::CreateTexture(device, "tex1.png");
		if (!tex)
			return 1;

		// create cube
		Object cube(mesh);
		cube.phi = D3DX_PI * 0.25f;
		cube.theta = D3DX_PI * 0.25f;

		// create camera
		Camera camera;
		camera.aspect = static_cast<float>(window->GetWidth()) / window->GetHeight();
		camera.pos.z = 5;

		// create shader
		VertexShader vs(device, "vs_transform.hlsl");
		if (!vs)
			return 1;
		if (!vs.Enable(device))
			return 1;

		// create light
		D3DLIGHT9 light;
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Ambient = D3DXCOLOR(1, 1, 1, 1) * 0.3f;
		light.Diffuse = D3DXCOLOR(1, 1, 1, 1);
		light.Specular = D3DXCOLOR(1, 1, 1, 1) * 0.6f;
		light.Direction = D3DXVECTOR3(-0.0f, -0.99f, -0.14f);
		hr = device->SetLight(0, &light);
		if (FAILED(hr))
			return 1;
		hr = device->LightEnable(0, true);
		if (FAILED(hr))
			return 1;

		// set normalized normal
		hr = device->SetRenderState(D3DRS_NORMALIZENORMALS, true); // default is false
		if (FAILED(hr))
			return 1;

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

			// set rendering state
			if (KeyIsDown('4')) hr = device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			else if (KeyIsDown('5')) hr = device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID); // default
			if (FAILED(hr))
				return 1;
			if (KeyIsDown('6')) hr = device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
			else if (KeyIsDown('7')) hr = device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD); // default
			if (FAILED(hr))
				return 1;
			if (KeyIsDown('8')) hr = device->SetRenderState(D3DRS_LIGHTING, true); // default
			else if (KeyIsDown('9')) hr = device->SetRenderState(D3DRS_LIGHTING, false);
			if (FAILED(hr))
				return 1;
			if (KeyIsDown('Z')) hr = device->SetRenderState(D3DRS_SPECULARENABLE, true);
			else if (KeyIsDown('X')) hr = device->SetRenderState(D3DRS_SPECULARENABLE, false); // default
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

			// set tex
			if (KeyIsDown('C'))
				cube.texture = nullptr;
			else if (KeyIsDown('V'))
				cube.texture = tex;

			// set shader
			if (KeyIsDown(VK_NUMPAD1))
			{
				hr = device->SetVertexShader(nullptr);
				if (FAILED(hr))
					return 1;
			}
			else if (KeyIsDown(VK_NUMPAD2))
			{
				if (!vs.Enable(device))
					return 1;
			}

			// set shader const
			D3DXMATRIX camera_transform = camera.ComputeViewTransform() * camera.ComputeProjectionTransform();
			D3DXMATRIX transform = cube.ComputeTransform() * camera_transform;
			hr = vs.GetCT()->SetMatrix(device, vs.GetCT()->GetConstantByName(nullptr, "transform"), &transform);
			if (FAILED(hr))
				return 1;

			// set camera and projection
			if (!camera.Transform(device))
				return 1;

			// render
			hr = device->BeginScene();
			if (FAILED(hr))
				return 1;
			if (!cube.Draw(device))
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