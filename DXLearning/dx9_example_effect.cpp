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

		// set tex in effect
		for (int par_index = 0; true; par_index++)
		{
			auto tex_handle = fx.GetFX()->GetParameterByName(nullptr, ("tex" + std::to_string(par_index)).c_str());
			if (tex_handle == nullptr)
				break;
			auto name_handle = fx.GetFX()->GetAnnotationByName(tex_handle, "name");
			if (name_handle == nullptr)
			{
				cout << "warn : tex" << par_index << " not have annotation name" << endl;
				continue;
			}
			const char* file_name;
			hr = fx.GetFX()->GetString(name_handle, &file_name);
			if (FAILED(hr))
			{
				cout << "warn : fail read tex" << par_index << " annotation name" << endl;
				continue;
			}
			auto tex = Texture::CreateTexture(device, file_name);
			hr = fx.GetFX()->SetTexture(tex_handle, tex->GetInterface());
			if (FAILED(hr))
			{
				cout << "warn : fail set tex" << par_index << " from " << file_name << endl;
				continue;
			}
		}

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

			// set technique
			D3DXHANDLE tech_new = tech;
			if (KeyIsDown(VK_NUMPAD1)) tech_new = fx.GetFX()->GetTechnique(0);
			else if (KeyIsDown(VK_NUMPAD2)) tech_new = fx.GetFX()->GetTechnique(1);
			else if (KeyIsDown(VK_NUMPAD3)) tech_new = fx.GetFX()->GetTechnique(2);
			else if (KeyIsDown(VK_NUMPAD4)) tech_new = fx.GetFX()->GetTechnique(3);
			else if (KeyIsDown(VK_NUMPAD5)) tech_new = fx.GetFX()->GetTechnique(4);
			else if (KeyIsDown(VK_NUMPAD6)) tech_new = fx.GetFX()->GetTechnique(5);
			else if (KeyIsDown(VK_NUMPAD7)) tech_new = fx.GetFX()->GetTechnique(6);
			else if (KeyIsDown(VK_NUMPAD8)) tech_new = fx.GetFX()->GetTechnique(7);
			if (tech_new != tech && tech_new != nullptr)
			{
				hr = fx.GetFX()->ValidateTechnique(tech_new);
				if (SUCCEEDED(hr))
					tech = tech_new;
				else
					cout << "not support technique !" << endl;
			}

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
			fx.GetFX()->SetMatrix(fx.GetFX()->GetParameterByName(nullptr, "obj_to_view_transform"), &to_view_transform);
			fx.GetFX()->SetMatrix(fx.GetFX()->GetParameterByName(nullptr, "obj_to_proj_transform"), &to_proj_transform);
			fx.GetFX()->SetFloat(fx.GetFX()->GetParameterByName(nullptr, "time"), t);

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