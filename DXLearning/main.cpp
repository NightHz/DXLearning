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
	auto mesh_cube = Dx9::Mesh::CreateCubeNormalColorTex1(device);
	if (!mesh_cube)
		return 1;
	auto mesh_teapot = Dx9::Mesh::CreateD3DXTeapot(device);
	mesh_teapot = Dx9::Mesh::UpdatePMesh(mesh_teapot);
	if (!mesh_teapot)
		return 1;
	auto mesh_tetrahedron = Dx9::Mesh::CreateTetrahedronNormalColor(device);
	if (!mesh_tetrahedron)
		return 1;
	auto mesh_plane = Dx9::Mesh::CreatePlaneNormal(device);
	if (!mesh_plane)
		return 1;
	auto mesh_text = Dx9::Mesh::CreateD3DXText(device, "Dx9 Sample by NightHz");
	if (!mesh_text)
		return 1;
	auto mesh_machete = Dx9::Mesh::CreateMeshNormalFromFile(device, "model/machete.obj");
	if (!mesh_machete)
		return 1;
	D3DXVECTOR3 min, max, center;
	float radius;
	if (!mesh_machete->ComputeBoundingBox(min, max))
		return 1;
	if (!mesh_machete->ComputeBoundingSphere(center, radius))
		return 1;
	cout << "machete bounding box : [(" << min.x << ", " << min.y << ", " << min.z << ")";
	cout << ", (" << max.x << ", " << max.y << ", " << max.z << ")]" << endl;
	cout << "machete bounding sphere : [(" << center.x << ", " << center.y << ", " << center.z << ")";
	cout << ", " << radius << "]" << endl;

	// create texture
	auto texture1 = Dx9::Texture::CreateTexture(device, "tex1.png");
	if (!texture1)
		return 1;

	// create cube
	Dx9::Object cube(mesh_cube);
	cube.texture = texture1;
	cube.phi = D3DX_PI * 0.25f;
	cube.theta = D3DX_PI * 0.25f;
	cube.x = 4;

	// create d3dx teapot
	Dx9::Object teapot(mesh_teapot);
	teapot.mat.Diffuse.a = 0.5f;
	teapot.x = 2;
	teapot.z = 4;
	teapot.sx = 0.6f;
	teapot.sy = 0.6f;
	teapot.sz = 0.6f;
	float teapot_progress = 1;

	// create small cubes
	Dx9::Object cubes_s[100];
	for (int i = 0; i < 100; i++)
	{
		cubes_s[i].mesh = mesh_cube;
		cubes_s[i].sx = 0.2f;
		cubes_s[i].sy = 0.2f;
		cubes_s[i].sz = 0.2f;
		cubes_s[i].y = -8;
		cubes_s[i].x = i / 10 - 4.5f;
		cubes_s[i].z = i % 10 - 4.5f;
	}

	// create tetrahedron
	Dx9::Object tetrahedron(mesh_tetrahedron);
	tetrahedron.x = 3;
	tetrahedron.y = -4;
	tetrahedron.sx = 3.0f;
	tetrahedron.sz = 3.0f;

	// create cube2
	Dx9::Object cube2(mesh_cube);
	cube2.phi = D3DX_PI * 0.25f;
	cube2.theta = D3DX_PI * 0.25f;
	cube2.x = -4;
	cube2.z = 2;

	// create cube3
	Dx9::Object cube3(mesh_cube);
	cube3.phi = D3DX_PI * 0.25f;
	cube3.theta = D3DX_PI * 0.25f;
	cube3.x = -2.5f;
	cube3.z = 2;
	cube3.y = 3;
	cube3.sx = 0.4f;
	cube3.sy = 0.4f;
	cube3.sz = 0.4f;

	// create mirror
	Dx9::Object mirror(mesh_plane);
	mirror.mat.Ambient = D3DXCOLOR(100, 100, 100, 1);
	mirror.x = -4;
	mirror.z = -1;
	mirror.sx = 3;
	mirror.sy = 3;

	// create ground
	Dx9::Object ground(mesh_plane);
	ground.mat.Diffuse = D3DXCOLOR(0.2f, 1, 1, 1);
	ground.phi = D3DX_PI * 0.5f;
	ground.theta = D3DX_PI * 0.5f;
	ground.x = -4;
	ground.z = 2;
	ground.y = -4;
	ground.sx = 2.5f;
	ground.sy = 2.5f;

	// create text
	Dx9::Object text(mesh_text);
	text.phi = D3DX_PI;
	text.x = 3;
	text.y = -2;
	text.z = 3;
	text.sx = 0.6f;
	text.sy = 0.6f;
	text.sz = 0.6f;

	// create machete
	Dx9::Object machete(mesh_machete);
	machete.sx = 0.005f;
	machete.sy = 0.005f;
	machete.sz = 0.005f;
	machete.x = 8;
	machete.y = 4;

	// create camera
	Dx9::Camera camera;
	camera.aspect = static_cast<float>(window.GetWidth()) / window.GetHeight();
	camera.pos.z = 8;

	// create light
	D3DLIGHT9 light;
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Ambient = D3DXCOLOR(1, 1, 1, 1) * 0.3f;
	light.Diffuse = D3DXCOLOR(1, 1, 1, 1);
	light.Specular = D3DXCOLOR(1, 1, 1, 1) * 0.6f;
	light.Direction = D3DXVECTOR3(-0.0f, -1.0f, -0.0f);
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

	// set blend
	hr = device->SetRenderState(D3DRS_ALPHABLENDENABLE, true); // default is false
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // default is D3DBLEND_ONE
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); // default is D3DBLEND_ZERO
	if (FAILED(hr))
		return 1;

	// set stencil
	//DWORD v;
	//device->GetRenderState(D3DRS_STENCILENABLE, &v);
	//cout << "D3DRS_STENCILENABLE : " << v << endl;
	hr = device->SetRenderState(D3DRS_STENCILENABLE, true); // default is false
	if (FAILED(hr))
		return 1;

	// set texture filter
	hr = device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	if (FAILED(hr))
		return 1;
	hr = device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	if (FAILED(hr))
		return 1;
	hr = device->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 4);
	if (FAILED(hr))
		return 1;
	hr = device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	if (FAILED(hr))
		return 1;

	cout << "finish setup" << endl;

	POINT mouse_pos, mouse_pos2;
	mouse_pos.x = window.GetWidth() / 2;
	mouse_pos.y = window.GetHeight() / 2;
	GetCursorPos(&mouse_pos2);
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
		if (KeyIsDown('Z'))
			hr = device->SetRenderState(D3DRS_SPECULARENABLE, true);
		else if (KeyIsDown('X'))
			hr = device->SetRenderState(D3DRS_SPECULARENABLE, false); // default
		if (FAILED(hr))
			return 1;

		// set texture filter
		if (KeyIsDown('C'))
		{
			hr = device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP); // default
			if (FAILED(hr))
				return 1;
			hr = device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP); // default
			if (FAILED(hr))
				return 1;
		}
		else if (KeyIsDown('V'))
		{
			hr = device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
			if (FAILED(hr))
				return 1;
			hr = device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
			if (FAILED(hr))
				return 1;
			hr = device->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0xffffffff);
			if (FAILED(hr))
				return 1;
		}
		else if (KeyIsDown('B'))
		{
			hr = device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			if (FAILED(hr))
				return 1;
			hr = device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			if (FAILED(hr))
				return 1;
		}
		else if (KeyIsDown('N'))
		{
			hr = device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
			if (FAILED(hr))
				return 1;
			hr = device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
			if (FAILED(hr))
				return 1;
		}

		// set camera and projection
		if (KeyIsDown('W')) camera.MoveFront(0.1f);
		else if (KeyIsDown('S')) camera.MoveBack(0.1f);
		if (KeyIsDown('A')) camera.MoveLeft(0.1f);
		else if (KeyIsDown('D')) camera.MoveRight(0.1f);
		if (KeyIsDown(VK_SPACE)) camera.MoveUp(0.1f);
		else if (KeyIsDown(VK_LSHIFT)) camera.MoveDown(0.1f);
		if (KeyIsDown('E')) camera.pos.x = camera.pos.y = 0;
		if (KeyIsDown(VK_MBUTTON))
		{
			POINT mouse_pos3;
			GetCursorPos(&mouse_pos3);
			camera.YawRight(0.003f * (mouse_pos3.x - mouse_pos2.x));
			camera.PitchDown(0.003f * (mouse_pos3.y - mouse_pos2.y));
			SetCursorPos(mouse_pos2.x, mouse_pos2.y);
		}
		else
			GetCursorPos(&mouse_pos2);
		if (!camera.Transform(device))
			return 1;

		// begin
		hr = device->BeginScene();
		if (FAILED(hr))
			return 1;

		// control
		static auto control_obj = &text;
		if (KeyIsDown(VK_NUMPAD1)) control_obj = &mirror;
		else if (KeyIsDown(VK_NUMPAD2)) control_obj = &ground;
		if (KeyIsDown('I')) control_obj->theta -= 0.05f;
		else if (KeyIsDown('K')) control_obj->theta += 0.05f;
		if (KeyIsDown('J')) control_obj->psi += 0.05f;
		else if (KeyIsDown('L')) control_obj->psi -= 0.05f;
		if (KeyIsDown('T')) control_obj->y += 0.1f;
		else if (KeyIsDown('G')) control_obj->y -= 0.1f;
		if (KeyIsDown('F')) control_obj->x += 0.1f;
		else if (KeyIsDown('H')) control_obj->x -= 0.1f;
		if (KeyIsDown('R')) control_obj->z += 0.1f;
		else if (KeyIsDown('Y')) control_obj->z -= 0.1f;

		// draw cube
		if (!cube.Draw(device))
			return 1;

		// draw small cubes
		for (int i = 0; i < 100; i++)
		{
			if (!cubes_s[i].Draw(device))
				return 1;
		}

		// adjust teapot
		if (KeyIsDown(VK_PRIOR)) teapot_progress = min(teapot_progress + 0.003f, 1);
		else if (KeyIsDown(VK_NEXT)) teapot_progress = max(teapot_progress - 0.003f, 0);
		if (!teapot.mesh->AdjustProgress(teapot_progress))
			cout << "failed : adjust teapot progress" << endl;

		// draw teapot
		if (!teapot.Draw(device))
			return 1;

		// draw tetrahedron
		if (!tetrahedron.Draw(device))
			return 1;

		// draw cube2
		if (!cube2.Draw(device))
			return 1;

		// draw cube3
		if (!cube3.Draw(device))
			return 1;

		// draw text
		if (!text.Draw(device))
			return 1;

		// draw machete
		if (!machete.Draw(device))
			return 1;

		// draw ground and update stencil
		hr = device->Clear(0, nullptr, D3DCLEAR_STENCIL, 0, 0, 0);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_STENCILREF, 0x1);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
		if (FAILED(hr))
			return 1;
		if (!ground.Draw(device))
			return 1;
		hr = device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
		if (FAILED(hr))
			return 1;

		// begin shadow
		// set stencil test
		hr = device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCRSAT);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
		if (FAILED(hr))
			return 1;
		// disable z test
		hr = device->SetRenderState(D3DRS_ZENABLE, false);
		if (FAILED(hr))
			return 1;
		// compute ground plane
		D3DXVECTOR4 g_n0(0, 0, 1, 0), g_n1;
		D3DXMATRIX g_transform = ground.ComputeTransform();
		D3DXVec4Transform(&g_n1, &g_n0, &g_transform);
		D3DXVECTOR3 g_n2(g_n1), g_n3;
		D3DXVec3Normalize(&g_n3, &g_n2);
		D3DXPLANE plane_ground(g_n3.x, g_n3.y, g_n3.z, -(ground.x * g_n3.x + ground.y * g_n3.y + ground.z * g_n3.z));

		// draw cube2 shadow
		if (!cube2.DrawShadow(device, D3DXVECTOR4(light.Direction, 0), plane_ground))
			return 1;

		// draw cube3 shadow
		if (!cube3.DrawShadow(device, D3DXVECTOR4(light.Direction, 0), plane_ground))
			return 1;

		// end shadow
		hr = device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_ZENABLE, true);
		if (FAILED(hr))
			return 1;

		// draw mirror and update stencil
		hr = device->Clear(0, nullptr, D3DCLEAR_STENCIL, 0, 0, 0);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_STENCILREF, 0x1);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
		if (FAILED(hr))
			return 1;
		if (!mirror.Draw(device))
			return 1;
		hr = device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
		if (FAILED(hr))
			return 1;

		// begin mirror
		// set stencil test
		hr = device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
		if (FAILED(hr))
			return 1;
		// set blend
		hr = device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		if (FAILED(hr))
			return 1;
		// set winding order
		hr = device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		if (FAILED(hr))
			return 1;
		// clear zbuffer
		hr = device->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		if (FAILED(hr))
			return 1;
		// compute mirror plane
		D3DXVECTOR4 n0(0, 0, 1, 0), n1;
		D3DXMATRIX transform = mirror.ComputeTransform();
		D3DXVec4Transform(&n1, &n0, &transform);
		D3DXVECTOR3 n2(n1), n3;
		D3DXVec3Normalize(&n3, &n2);
		D3DXPLANE plane_mirror(n3.x, n3.y, n3.z, -(mirror.x * n3.x + mirror.y * n3.y + mirror.z * n3.z));
		// set reflect
		if (!camera.TransformReflect(device, plane_mirror))
			return 1;

		// draw cube2 mirror
		if (!cube2.Draw(device))
			return 1;

		// draw cube3 mirror
		if (!cube3.Draw(device))
			return 1;

		// end mirror
		hr = device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		if (FAILED(hr))
			return 1;
		hr = device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		if (FAILED(hr))
			return 1;

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