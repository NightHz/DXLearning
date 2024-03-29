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
		auto mesh_teapot = Mesh::CreateD3DXTeapot(device);
		if (!mesh_teapot)
			return 1;

		// create texture
		auto tex = Texture::CreateTexture(device, "tex1.png");
		if (!tex)
			return 1;
		auto tex_cartoon = Texture::CreateTexture(device, "tex_cartoon.png");
		if (!tex_cartoon)
			return 1;

		// create cube
		Object cube(mesh);
		//cube.phi = D3DX_PI * 0.25f;
		//cube.theta = D3DX_PI * 0.25f;

		// create camera
		Camera camera;
		camera.aspect = static_cast<float>(window->GetWidth()) / window->GetHeight();
		camera.pos.z = 5;

		// create shader
		int current_vs = 1;
		VertexShader vs2(device, "vs_transform.hlsl");
		if (!vs2)
			return 1;
		VertexShader vs3(device, "vs_light.hlsl");
		if (!vs3)
			return 1;
		VertexShader vs4(device, "vs_cartoon.hlsl");
		if (!vs4)
			return 1;
		VertexShader vs4_2(device, "vs_outline.hlsl");
		if (!vs4_2)
			return 1;
		PixelShader ps5(device, "ps_tex_rotate.hlsl");
		if (!ps5)
			return 1;

		// create light
		D3DLIGHT9 light;
		D3DXVECTOR4 light_dir = D3DXVECTOR4(-0.0f, -0.99f, -0.14f, 0);
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Ambient = D3DXCOLOR(1, 1, 1, 1) * 0.3f;
		light.Diffuse = D3DXCOLOR(1, 1, 1, 1);
		light.Specular = D3DXCOLOR(1, 1, 1, 1) * 0.6f;
		light.Direction = D3DXVECTOR3(light_dir.x, light_dir.y, light_dir.z);
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
			if (KeyIsDown('Z')) hr = device->SetRenderState(D3DRS_LIGHTING, true); // default
			else if (KeyIsDown('X')) hr = device->SetRenderState(D3DRS_LIGHTING, false);
			if (FAILED(hr))
				return 1;
			if (KeyIsDown('C')) hr = device->SetRenderState(D3DRS_SPECULARENABLE, true);
			else if (KeyIsDown('V')) hr = device->SetRenderState(D3DRS_SPECULARENABLE, false); // default
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
			if (KeyIsDown('B'))
				cube.texture = nullptr;
			else if (KeyIsDown('N'))
				cube.texture = tex;

			// set shader
			if (KeyIsDown(VK_NUMPAD1))
			{
				hr = device->SetVertexShader(nullptr);
				if (FAILED(hr))
					return 1;
				hr = device->SetPixelShader(nullptr);
				if (FAILED(hr))
					return 1;
				cube.texture = nullptr;
				current_vs = 1;
			}
			else if (KeyIsDown(VK_NUMPAD2))
			{
				hr = device->SetPixelShader(nullptr);
				if (FAILED(hr))
					return 1;
				if (!vs2.Enable(device))
					return 1;
				cube.mesh = mesh;
				cube.texture = nullptr;
				current_vs = 2;
			}
			else if (KeyIsDown(VK_NUMPAD3))
			{
				hr = device->SetPixelShader(nullptr);
				if (FAILED(hr))
					return 1;
				if (!vs3.Enable(device))
					return 1;
				cube.mesh = mesh_teapot;
				cube.texture = nullptr;
				current_vs = 3;
			}
			else if (KeyIsDown(VK_NUMPAD4))
			{
				hr = device->SetPixelShader(nullptr);
				if (FAILED(hr))
					return 1;
				if (!vs4.Enable(device))
					return 1;
				cube.mesh = mesh_teapot;
				cube.texture = tex_cartoon;
				current_vs = 4;
			}
			else if (KeyIsDown(VK_NUMPAD5))
			{
				hr = device->SetVertexShader(nullptr);
				if (FAILED(hr))
					return 1;
				if (!vs2.Enable(device))
					return 1;
				if (!ps5.Enable(device))
					return 1;
				cube.mesh = mesh;
				cube.texture = tex;
				current_vs = 5;
			}

			// set camera and projection
			if (!camera.Transform(device))
				return 1;

			// set shader const
			D3DXMATRIX obj_transform = cube.ComputeTransform();
			D3DXMATRIX view_transform = camera.ComputeViewTransform();
			D3DXMATRIX proj_transform = camera.ComputeProjectionTransform();
			D3DXMATRIX to_view_transform = obj_transform * view_transform;
			D3DXMATRIX to_proj_transform = to_view_transform * proj_transform;
			static float t = 0;
			float dt = window->fps_counter.GetLastDeltatime() / 1000.0f;
			t += dt;
			if (current_vs == 2)
			{
				vs2.GetCT()->SetDefaults(device);
				vs2.GetCT()->SetMatrix(device, vs2.GetCT()->GetConstantByName(nullptr, "transform"), &to_proj_transform);
				vs2.GetCT()->SetFloat(device, vs2.GetCT()->GetConstantByName(nullptr, "time"), t);
			}
			else if (current_vs == 3)
			{
				vs3.GetCT()->SetDefaults(device);
				vs3.GetCT()->SetMatrix(device, vs3.GetCT()->GetConstantByName(nullptr, "world_to_view_transform"), &view_transform);
				vs3.GetCT()->SetMatrix(device, vs3.GetCT()->GetConstantByName(nullptr, "obj_to_view_transform"), &to_view_transform);
				vs3.GetCT()->SetMatrix(device, vs3.GetCT()->GetConstantByName(nullptr, "obj_to_proj_transform"), &to_proj_transform);
				vs3.GetCT()->SetVector(device, vs3.GetCT()->GetConstantByName(nullptr, "light_dir"), &light_dir);
				vs3.GetCT()->SetFloat(device, vs3.GetCT()->GetConstantByName(nullptr, "time"), t);
				vs3.GetCT()->SetFloat(device, vs3.GetCT()->GetConstantByName(nullptr, "delta_time"), dt);
				DWORD v;
				hr = device->GetRenderState(D3DRS_LIGHTING, &v);
				if (FAILED(hr))
					return 1;
				vs3.GetCT()->SetBool(device, vs3.GetCT()->GetConstantByName(nullptr, "light_enable"), v);
				hr = device->GetRenderState(D3DRS_SPECULARENABLE, &v);
				if (FAILED(hr))
					return 1;
				vs3.GetCT()->SetBool(device, vs3.GetCT()->GetConstantByName(nullptr, "specular_enable"), v);
			}
			else if (current_vs == 4)
			{
				// render outline using back face
				if (!vs4_2.Enable(device))
					return 1;
				hr = device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
				if (FAILED(hr))
					return 1;
				cube.texture = nullptr;
				vs4_2.GetCT()->SetDefaults(device);
				vs4_2.GetCT()->SetMatrix(device, vs4_2.GetCT()->GetConstantByName(nullptr, "obj_to_proj_transform"), &to_proj_transform);
				hr = device->BeginScene();
				if (FAILED(hr))
					return 1;
				if (!cube.Draw(device))
					return 1;
				hr = device->EndScene();
				if (FAILED(hr))
					return 1;
				hr = device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
				if (FAILED(hr))
					return 1;

				if (!vs4.Enable(device))
					return 1;
				cube.texture = tex_cartoon;
				vs4.GetCT()->SetDefaults(device);
				vs4.GetCT()->SetMatrix(device, vs4.GetCT()->GetConstantByName(nullptr, "world_to_view_transform"), &view_transform);
				vs4.GetCT()->SetMatrix(device, vs4.GetCT()->GetConstantByName(nullptr, "obj_to_view_transform"), &to_view_transform);
				vs4.GetCT()->SetMatrix(device, vs4.GetCT()->GetConstantByName(nullptr, "obj_to_proj_transform"), &to_proj_transform);
				vs4.GetCT()->SetVector(device, vs4.GetCT()->GetConstantByName(nullptr, "light_dir"), &light_dir);
			}
			else if (current_vs == 5)
			{
				vs2.GetCT()->SetDefaults(device);
				vs2.GetCT()->SetMatrix(device, vs2.GetCT()->GetConstantByName(nullptr, "transform"), &to_proj_transform);
				vs2.GetCT()->SetFloat(device, vs2.GetCT()->GetConstantByName(nullptr, "time"), 4.8f);
				ps5.GetCT()->SetDefaults(device);
				ps5.GetCT()->SetFloat(device, ps5.GetCT()->GetConstantByName(nullptr, "time"), t);
				D3DXHANDLE handle = ps5.GetCT()->GetConstantByName(nullptr, "tex1");
				if (handle != nullptr)
				{
					D3DXCONSTANT_DESC desc;
					UINT count;
					hr = ps5.GetCT()->GetConstantDesc(handle, &desc, &count);
					if (FAILED(hr))
						return 1;
					hr = device->SetTexture(desc.RegisterIndex, tex->GetInterface());
					if (FAILED(hr))
						return 1;
					hr = device->SetSamplerState(desc.RegisterIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
					if (FAILED(hr))
						return 1;
					hr = device->SetSamplerState(desc.RegisterIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
					if (FAILED(hr))
						return 1;
					hr = device->SetSamplerState(desc.RegisterIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
					if (FAILED(hr))
						return 1;
				}
			}

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