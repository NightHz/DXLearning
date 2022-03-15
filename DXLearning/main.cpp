#include <iostream>
#include "window.h"
#include "input.h"
#include "dx9.h"
#include <unordered_map>

using std::cout;
using std::endl;
using std::cin;

template <typename T>
using library = std::unordered_map<std::string, std::shared_ptr<T>>;

library<Dx9::Mesh> dx9_meshes;
library<Dx9::Texture> dx9_textures;
library<Dx9::Object> dx9_objects;
library<Dx9::Particles> dx9_particles;
std::shared_ptr<Dx9::Camera> camera;
std::unordered_map<std::string, int> control_value;

int dx9_setup(SimpleWindowWithFC* window, IDirect3DDevice9* device)
{
	// create mesh
	dx9_meshes["cube"] = Dx9::Mesh::CreateCubeNormalColorTex1(device);
	if (!dx9_meshes["cube"])
		return 1;
	dx9_meshes["teapot"] = Dx9::Mesh::CreateD3DXTeapot(device);
	if (!dx9_meshes["teapot"])
		return 1;
	if (!dx9_meshes["teapot"]->UpdatePMesh())
		return 1;
	dx9_meshes["tetrahedron"] = Dx9::Mesh::CreateTetrahedronNormalColor(device);
	if (!dx9_meshes["tetrahedron"])
		return 1;
	dx9_meshes["plane"] = Dx9::Mesh::CreatePlaneNormal(device);
	if (!dx9_meshes["plane"])
		return 1;
	dx9_meshes["text"] = Dx9::Mesh::CreateD3DXText(device, "Dx9 Sample by NightHz");
	if (!dx9_meshes["text"])
		return 1;
	dx9_meshes["machete"] = Dx9::Mesh::CreateMeshNormalFromFile(device, "model/machete.obj");
	if (!dx9_meshes["machete"])
		return 1;
	dx9_meshes["machete_box"] = Dx9::Mesh::CreateD3DXCube(device);
	if (!dx9_meshes["machete_box"])
		return 1;
	dx9_meshes["machete_sphere"] = Dx9::Mesh::CreateD3DXSphere(device);
	if (!dx9_meshes["machete_sphere"])
		return 1;
	dx9_meshes["terrain"] = Dx9::Mesh::CreateTerrainRandom(device);
	if (!dx9_meshes["terrain"])
		return 1;

	// create texture
	dx9_textures["texture1"] = Dx9::Texture::CreateTexture(device, "tex1.png");
	if (!dx9_textures["texture1"])
		return 1;
	dx9_textures["snow"] = Dx9::Texture::CreateTexture(device, "tex2.png");
	if (!dx9_textures["snow"])
		return 1;

	// create cube
	dx9_objects["cube"] = std::make_shared<Dx9::Object>(dx9_meshes["cube"]);
	auto& cube = *dx9_objects["cube"];
	cube.texture = dx9_textures["texture1"];
	cube.phi = D3DX_PI * 0.25f;
	cube.theta = D3DX_PI * 0.25f;
	cube.x = 4;

	// create d3dx teapot
	dx9_objects["teapot"] = std::make_shared<Dx9::Object>(dx9_meshes["teapot"]);
	auto& teapot = *dx9_objects["teapot"];
	teapot.mat.Diffuse.a = 0.5f;
	teapot.x = 2;
	teapot.z = 4;
	teapot.sx = 0.6f;
	teapot.sy = 0.6f;
	teapot.sz = 0.6f;

	// create small cubes
	dx9_objects["cubes_s"] = std::shared_ptr<Dx9::Object>(new Dx9::Object[100], [](Dx9::Object* p) { delete[] p; });
	auto cubes_s = dx9_objects["cubes_s"];
	for (int i = 0; i < 100; i++)
	{
		cubes_s.get()[i].mesh = dx9_meshes["cube"];
		cubes_s.get()[i].sx = 0.2f;
		cubes_s.get()[i].sy = 0.2f;
		cubes_s.get()[i].sz = 0.2f;
		cubes_s.get()[i].y = -8;
		cubes_s.get()[i].x = i / 10 - 4.5f;
		cubes_s.get()[i].z = i % 10 - 4.5f;
	}

	// create tetrahedron
	dx9_objects["tetrahedron"] = std::make_shared<Dx9::Object>(dx9_meshes["tetrahedron"]);
	auto& tetrahedron = *dx9_objects["tetrahedron"];
	tetrahedron.x = 3;
	tetrahedron.y = -4;
	tetrahedron.sx = 3.0f;
	tetrahedron.sz = 3.0f;

	// create cube2
	dx9_objects["cube2"] = std::make_shared<Dx9::Object>(dx9_meshes["cube"]);
	auto& cube2 = *dx9_objects["cube2"];
	cube2.phi = D3DX_PI * 0.25f;
	cube2.theta = D3DX_PI * 0.25f;
	cube2.x = -4;
	cube2.z = 2;

	// create cube3
	dx9_objects["cube3"] = std::make_shared<Dx9::Object>(dx9_meshes["cube"]);
	auto& cube3 = *dx9_objects["cube3"];
	cube3.phi = D3DX_PI * 0.25f;
	cube3.theta = D3DX_PI * 0.25f;
	cube3.x = -2.5f;
	cube3.z = 2;
	cube3.y = 3;
	cube3.sx = 0.4f;
	cube3.sy = 0.4f;
	cube3.sz = 0.4f;

	// create mirror
	dx9_objects["mirror"] = std::make_shared<Dx9::Object>(dx9_meshes["plane"]);
	auto& mirror = *dx9_objects["mirror"];
	mirror.mat.Ambient = D3DXCOLOR(100, 100, 100, 1);
	mirror.x = -4;
	mirror.z = -1;
	mirror.sx = 3;
	mirror.sy = 3;

	// create ground
	dx9_objects["ground"] = std::make_shared<Dx9::Object>(dx9_meshes["plane"]);
	auto& ground = *dx9_objects["ground"];
	ground.mat.Diffuse = D3DXCOLOR(0.2f, 1, 1, 1);
	ground.phi = D3DX_PI * 0.5f;
	ground.theta = D3DX_PI * 0.5f;
	ground.x = -4;
	ground.z = 2;
	ground.y = -4;
	ground.sx = 2.5f;
	ground.sy = 2.5f;

	// create text
	dx9_objects["text"] = std::make_shared<Dx9::Object>(dx9_meshes["text"]);
	auto& text = *dx9_objects["text"];
	text.phi = D3DX_PI;
	text.x = 3;
	text.y = -2;
	text.z = 3;
	text.sx = 0.6f;
	text.sy = 0.6f;
	text.sz = 0.6f;

	// create machete
	dx9_objects["machete"] = std::make_shared<Dx9::Object>(dx9_meshes["machete"]);
	auto& machete = *dx9_objects["machete"];
	machete.sx = 0.005f;
	machete.sy = 0.005f;
	machete.sz = 0.005f;
	machete.x = 8;
	machete.y = 4;

	// create machete bounding box and bounding sphere
	D3DXVECTOR3 min, max, center;
	float radius;
	if (!dx9_meshes["machete"]->ComputeBoundingBox(min, max))
		return 1;
	if (!dx9_meshes["machete"]->ComputeBoundingSphere(center, radius))
		return 1;
	cout << "machete bounding box : [(" << min.x << ", " << min.y << ", " << min.z << ")";
	cout << ", (" << max.x << ", " << max.y << ", " << max.z << ")]" << endl;
	cout << "machete bounding sphere : [(" << center.x << ", " << center.y << ", " << center.z << ")";
	cout << ", " << radius << "]" << endl;
	dx9_objects["machete_box"] = std::make_shared<Dx9::Object>(dx9_meshes["machete_box"]);
	auto& machete_box = *dx9_objects["machete_box"];
	machete_box.mat.Diffuse.a = 0.5f;
	machete_box.sx = machete.sx * (max.x - min.x);
	machete_box.sy = machete.sy * (max.y - min.y);
	machete_box.sz = machete.sz * (max.z - min.z);
	machete_box.x = machete.x + (max.x + min.x) * 0.5f * machete.sx;
	machete_box.y = machete.y + (max.y + min.y) * 0.5f * machete.sy;
	machete_box.z = machete.z + (max.z + min.z) * 0.5f * machete.sz;
	dx9_objects["machete_sphere"] = std::make_shared<Dx9::Object>(dx9_meshes["machete_sphere"]);
	auto& machete_sphere = *dx9_objects["machete_sphere"];
	machete_sphere.mat.Diffuse.a = 0.5f;
	machete_sphere.sx = machete.sx * radius;
	machete_sphere.sy = machete.sy * radius;
	machete_sphere.sz = machete.sz * radius;
	machete_sphere.x = machete.x + center.x * machete.sx;
	machete_sphere.y = machete.y + center.y * machete.sy;
	machete_sphere.z = machete.z + center.z * machete.sz;

	// create terrain
	dx9_objects["terrain"] = std::make_shared<Dx9::Object>(dx9_meshes["terrain"]);
	auto& terrain = *dx9_objects["terrain"];
	terrain.y = -18;

	// create particles
	dx9_particles["snow"] = std::make_shared<Dx9::SnowParticles>(device);
	Dx9::SnowParticles& snow = *dynamic_cast<Dx9::SnowParticles*>(dx9_particles["snow"].get());
	if (!snow.IsAlive())
		return 1;
	snow.texture = dx9_textures["snow"];
	snow.y = -18;
	snow.range_min.x = -20;
	snow.range_max.x = 20;
	snow.range_min.z = -20;
	snow.range_max.z = 20;
	snow.range_min.y = -5;
	snow.range_max.y = 8;
	snow.emit_rate = 3000;

	// create camera
	camera = std::make_shared<Dx9::Camera>();
	camera->aspect = static_cast<float>(window->GetWidth()) / window->GetHeight();
	camera->pos.z = 8;

	HRESULT hr = 0;
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

	// set point sprite
	hr = device->SetRenderState(D3DRS_POINTSPRITEENABLE, true); // default is false
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_POINTSCALEENABLE, true); // default is false
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_POINTSIZE, Dx9::float_to_DWORD(0.05f));
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_POINTSIZE_MIN, Dx9::float_to_DWORD(0.0f));
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_POINTSIZE_MAX, Dx9::float_to_DWORD(16.0f));
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_POINTSCALE_A, Dx9::float_to_DWORD(0.0f));
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_POINTSCALE_B, Dx9::float_to_DWORD(0.0f));
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_POINTSCALE_C, Dx9::float_to_DWORD(1.0f));
	if (FAILED(hr))
		return 1;

	return 0;
}

int dx9_control(IDirect3DDevice9* device)
{
	HRESULT hr = 0;
	static bool first = true;
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

	// control camera
	if (KeyIsDown('W')) camera->MoveFront(0.1f);
	else if (KeyIsDown('S')) camera->MoveBack(0.1f);
	if (KeyIsDown('A')) camera->MoveLeft(0.1f);
	else if (KeyIsDown('D')) camera->MoveRight(0.1f);
	if (KeyIsDown(VK_SPACE)) camera->MoveUp(0.1f);
	else if (KeyIsDown(VK_LSHIFT)) camera->MoveDown(0.1f);
	if (KeyIsDown('E')) camera->pos.x = camera->pos.y = 0;
	static POINT mouse_pos;
	if (first)
		GetCursorPos(&mouse_pos);
	if (KeyIsDown(VK_MBUTTON))
	{
		POINT mouse_pos2;
		GetCursorPos(&mouse_pos2);
		camera->YawRight(0.003f * (mouse_pos2.x - mouse_pos.x));
		camera->PitchDown(0.003f * (mouse_pos2.y - mouse_pos.y));
		SetCursorPos(mouse_pos.x, mouse_pos.y);
	}
	else
		GetCursorPos(&mouse_pos);

	// control obj
	static auto control_obj = dx9_objects["text"];
	if (KeyIsDown(VK_NUMPAD1)) control_obj = dx9_objects["mirror"];
	else if (KeyIsDown(VK_NUMPAD2)) control_obj = dx9_objects["ground"];
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

	// adjust teapot
	static float teapot_progress = 1;
	if (KeyIsDown(VK_F1)) teapot_progress = min(teapot_progress + 0.003f, 1);
	else if (KeyIsDown(VK_F2)) teapot_progress = max(teapot_progress - 0.003f, 0);
	if (!dx9_objects["teapot"]->mesh->AdjustProgress(teapot_progress))
		cout << "failed : adjust teapot progress" << endl;

	// adjust show machete bounding
	if (first)
		control_value["show_machete_bounding"] = 1;
	if (KeyIsDown(VK_F3)) control_value["show_machete_bounding"] = 0;
	else if (KeyIsDown(VK_F4)) control_value["show_machete_bounding"] = 1;
	else if (KeyIsDown(VK_F5)) control_value["show_machete_bounding"] = 2;

	// show info
	if (KeyIsDown(VK_F6)) cout << "snow particles count : " << dx9_particles["snow"]->GetParticlesCount() << endl;

	first = false;
	return 0;
}

int dx9_render_mirror(IDirect3DDevice9* device)
{
	HRESULT hr = 0;
	// begin mirror
	// draw mirror and update stencil
	hr = device->Clear(0, nullptr, D3DCLEAR_STENCIL, 0, 0, 0);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_STENCILENABLE, true);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_STENCILREF, 0x1);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
	if (FAILED(hr))
		return 1;
	if (!dx9_objects["mirror"]->Draw(device))
		return 1;

	// set stencil test
	hr = device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
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
	auto& mirror = *dx9_objects["mirror"];
	D3DXVECTOR4 n0(0, 0, 1, 0), n1;
	D3DXMATRIX transform = mirror.ComputeTransform();
	D3DXVec4Transform(&n1, &n0, &transform);
	D3DXVECTOR3 n2(n1), n3;
	D3DXVec3Normalize(&n3, &n2);
	D3DXPLANE plane_mirror(n3.x, n3.y, n3.z, -(mirror.x * n3.x + mirror.y * n3.y + mirror.z * n3.z));
	// set reflect
	if (!camera->TransformReflect(device, plane_mirror))
		return 1;

	// draw cube2 mirror
	if (!dx9_objects["cube2"]->Draw(device))
		return 1;
	// draw cube3 mirror
	if (!dx9_objects["cube3"]->Draw(device))
		return 1;

	// end mirror
	hr = device->SetRenderState(D3DRS_STENCILENABLE, false);
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
	if (!camera->Transform(device))
		return 1;

	return 0;
}

int dx9_render_shadow(IDirect3DDevice9* device)
{
	HRESULT hr = 0;
	// begin shadow
	// draw ground and update stencil
	hr = device->Clear(0, nullptr, D3DCLEAR_STENCIL, 0, 0, 0);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_STENCILENABLE, true);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_STENCILREF, 0x1);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	if (FAILED(hr))
		return 1;
	if (!dx9_objects["ground"]->Draw(device))
		return 1;

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
	auto& ground = *dx9_objects["ground"];
	D3DXVECTOR4 g_n0(0, 0, 1, 0), g_n1;
	D3DXMATRIX g_transform = ground.ComputeTransform();
	D3DXVec4Transform(&g_n1, &g_n0, &g_transform);
	D3DXVECTOR3 g_n2(g_n1), g_n3;
	D3DXVec3Normalize(&g_n3, &g_n2);
	D3DXPLANE plane_ground(g_n3.x, g_n3.y, g_n3.z, -(ground.x * g_n3.x + ground.y * g_n3.y + ground.z * g_n3.z));
	// get light
	D3DLIGHT9 light;
	hr = device->GetLight(0, &light);
	if (FAILED(hr))
		return 1;

	// draw cube2 shadow
	if (!dx9_objects["cube2"]->DrawShadow(device, D3DXVECTOR4(light.Direction, 0), plane_ground))
		return 1;
	// draw cube3 shadow
	if (!dx9_objects["cube3"]->DrawShadow(device, D3DXVECTOR4(light.Direction, 0), plane_ground))
		return 1;

	// end shadow
	hr = device->SetRenderState(D3DRS_STENCILENABLE, false);
	if (FAILED(hr))
		return 1;
	hr = device->SetRenderState(D3DRS_ZENABLE, true);
	if (FAILED(hr))
		return 1;

	return 0;
}

int dx9_render_particles(IDirect3DDevice9* device)
{
	// draw snow
	if (!dx9_particles["snow"]->Draw(device))
		return 1;

	return 0;
}

int dx9_render(IDirect3DDevice9* device)
{
	HRESULT hr = 0;
	// begin
	hr = device->BeginScene();
	if (FAILED(hr))
		return 1;
	// draw cube
	if (!dx9_objects["cube"]->Draw(device))
		return 1;
	// draw small cubes
	for (int i = 0; i < 100; i++)
	{
		if (!dx9_objects["cubes_s"].get()[i].Draw(device))
			return 1;
	}
	// draw teapot
	if (!dx9_objects["teapot"]->Draw(device))
		return 1;
	// draw tetrahedron
	if (!dx9_objects["tetrahedron"]->Draw(device))
		return 1;
	// draw cube2
	if (!dx9_objects["cube2"]->Draw(device))
		return 1;
	// draw cube3
	if (!dx9_objects["cube3"]->Draw(device))
		return 1;
	// draw text
	if (!dx9_objects["text"]->Draw(device))
		return 1;
	// draw machete
	if (!dx9_objects["machete"]->Draw(device))
		return 1;
	// draw machete bounding
	if (control_value["show_machete_bounding"] == 1)
	{
		if (!dx9_objects["machete_box"]->Draw(device))
			return 1;
	}
	else if (control_value["show_machete_bounding"] == 2)
	{
		if (!dx9_objects["machete_sphere"]->Draw(device))
			return 1;
	}
	// draw terrain
	if (!dx9_objects["terrain"]->Draw(device))
		return 1;

	// special render
	// draw particles
	if (dx9_render_particles(device) != 0)
		return 1;
	// draw shadow
	if (dx9_render_shadow(device) != 0)
		return 1;
	// draw mirror
	if (dx9_render_mirror(device) != 0)
		return 1;

	// end
	hr = device->EndScene();
	if (FAILED(hr))
		return 1;

	return 0;
}

int dx9_example()
{
	HRESULT hr;

	SimpleWindowWithFC* window = new SimpleWindowWithFC(GetModuleHandle(nullptr), 800, 600, "dx9");
	if (!window->CheckWindowState())
		return 1;
	cout << "finish create window" << endl;

	IDirect3DDevice9* device = Dx9::CreateSimpleDx9Device(window);
	if (!device)
		return 1;
	cout << "finish create dx9 device" << endl;

	if (dx9_setup(window, device) != 0)
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
		if (dx9_control(device) != 0)
			return 1;

		// set camera and projection
		if (!camera->Transform(device))
			return 1;

		// update particles
		unsigned int dt = min(window->fps_counter.GetLastDeltatime(), 1000);
		if (!dx9_particles["snow"]->Present(dt))
			return 1;

		// render
		if (dx9_render(device) != 0)
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
	cout << "finish release" << endl;

	return 0;
}

int main()
{
	cout << "DirectX learning ..." << endl;
	return dx9_example();
}