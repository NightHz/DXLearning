#include <iostream>
#include "window.h"
#include "input.h"
#include "dx9.h"
#include <map>
#include <set>

using std::cout;
using std::endl;
using std::cin;

int dx9_example_1_create_mesh(IDirect3DDevice9* device, std::map<std::string, std::shared_ptr<Dx9::Mesh>>& meshes)
{
	meshes["cube"] = Dx9::Mesh::CreateCubeNormalColorTex1(device);
	meshes["teapot"] = Dx9::Mesh::CreateD3DXTeapot(device);
	meshes["tetrahedron"] = Dx9::Mesh::CreateTetrahedronNormalColor(device);
	meshes["plane"] = Dx9::Mesh::CreatePlaneNormal(device);
	meshes["text"] = Dx9::Mesh::CreateD3DXText(device, "Dx9 Sample by NightHz");
	meshes["machete"] = Dx9::Mesh::CreateMeshNormalFromFile(device, "model/machete.obj");
	meshes["machete_box"] = Dx9::Mesh::CreateD3DXCube(device);
	meshes["machete_sphere"] = Dx9::Mesh::CreateD3DXSphere(device);
	meshes["terrain"] = Dx9::Mesh::CreateTerrainRandom(device);
	for (auto& p : meshes)
	{
		if (!p.second)
			return 1;
	}
	if (!meshes["teapot"]->UpdatePMesh())
		return 1;
	return 0;
}
int dx9_example_2_create_texture(IDirect3DDevice9* device, std::map<std::string, std::shared_ptr<Dx9::Texture>>& textures)
{
	textures["tex1"] = Dx9::Texture::CreateTexture(device, "tex1.png");
	textures["snow"] = Dx9::Texture::CreateTexture(device, "tex2.png");
	for (auto& p : textures)
	{
		if (!p.second)
			return 1;
	}
	return 0;
}
int dx9_example_3_create_object(std::map<std::string, std::shared_ptr<Dx9::Object>>& objs,
	std::map<std::string, int>& draw_order,
	std::set<std::string>& draw_set,
	std::set<std::string>& draw_mirror_set,
	std::set<std::string>& draw_shadow_set,
	std::map<std::string, std::shared_ptr<Dx9::Mesh>>& meshes,
	std::map<std::string, std::shared_ptr<Dx9::Texture>>& textures)
{
	// create cube
	auto cube = std::make_shared<Dx9::Object>(meshes["cube"]);
	cube->texture = textures["tex1"];
	cube->phi = D3DX_PI * 0.25f;
	cube->theta = D3DX_PI * 0.25f;
	cube->x = 4;
	objs["cube"] = cube;
	draw_order["cube"] = 10;
	draw_set.insert("cube");

	// create d3dx teapot
	auto teapot = std::make_shared<Dx9::Object>(meshes["teapot"]);
	teapot->mat.Diffuse.a = 0.5f;
	teapot->x = 2;
	teapot->z = 4;
	teapot->sx = 0.6f;
	teapot->sy = 0.6f;
	teapot->sz = 0.6f;
	objs["teapot"] = teapot;
	draw_order["teapot"] = 20;
	draw_set.insert("teapot");

	// create small cubes
	std::string cube_s_name = "small_cube";
	for (int i = 0; i < 100; i++)
	{
		auto cube_s = std::make_shared<Dx9::Object>(meshes["cube"]);
		cube_s->sx = 0.2f;
		cube_s->sy = 0.2f;
		cube_s->sz = 0.2f;
		cube_s->y = -8;
		cube_s->x = i / 10 - 4.5f;
		cube_s->z = i % 10 - 4.5f;
		std::string id = cube_s_name + std::to_string(i);
		objs[id] = cube_s;
		draw_order[id] = 0;
		draw_set.insert(id);
	}

	// create tetrahedron
	auto tetrahedron = std::make_shared<Dx9::Object>(meshes["tetrahedron"]);
	tetrahedron->x = 3;
	tetrahedron->y = -4;
	tetrahedron->sx = 3.0f;
	tetrahedron->sz = 3.0f;
	objs["tetrahedron"] = tetrahedron;
	draw_order["tetrahedron"] = 0;
	draw_set.insert("tetrahedron");

	// create cube2
	auto cube2 = std::make_shared<Dx9::Object>(meshes["cube"]);
	cube2->phi = D3DX_PI * 0.25f;
	cube2->theta = D3DX_PI * 0.25f;
	cube2->x = -4;
	cube2->z = 2;
	objs["cube2"] = cube2;
	draw_order["cube2"] = 0;
	draw_set.insert("cube2");
	draw_mirror_set.insert("cube2");
	draw_shadow_set.insert("cube2");

	// create cube3
	auto cube3 = std::make_shared<Dx9::Object>(meshes["cube"]);
	cube3->phi = D3DX_PI * 0.25f;
	cube3->theta = D3DX_PI * 0.25f;
	cube3->x = -2.5f;
	cube3->z = 2;
	cube3->y = 3;
	cube3->sx = 0.4f;
	cube3->sy = 0.4f;
	cube3->sz = 0.4f;
	objs["cube3"] = cube3;
	draw_order["cube3"] = 0;
	draw_set.insert("cube3");
	draw_mirror_set.insert("cube3");
	draw_shadow_set.insert("cube3");

	// create mirror
	auto mirror = std::make_shared<Dx9::Object>(meshes["plane"]);
	mirror->mat.Ambient = D3DXCOLOR(100, 100, 100, 1);
	mirror->x = -4;
	mirror->z = -1;
	mirror->sx = 3;
	mirror->sy = 3;
	objs["mirror"] = mirror;
	draw_order["mirror"] = 0;
	draw_set.insert("mirror");

	// create ground
	auto ground = std::make_shared<Dx9::Object>(meshes["plane"]);
	ground->mat.Diffuse = D3DXCOLOR(0.2f, 1, 1, 1);
	ground->phi = D3DX_PI * 0.5f;
	ground->theta = D3DX_PI * 0.5f;
	ground->x = -4;
	ground->z = 2;
	ground->y = -4;
	ground->sx = 2.5f;
	ground->sy = 2.5f;
	objs["ground"] = ground;
	draw_order["ground"] = 0;
	draw_set.insert("ground");

	// create text
	auto text = std::make_shared<Dx9::Object>(meshes["text"]);
	text->phi = D3DX_PI;
	text->x = 3;
	text->y = -2;
	text->z = 3;
	text->sx = 0.6f;
	text->sy = 0.6f;
	text->sz = 0.6f;
	objs["text"] = text;
	draw_order["text"] = 0;
	draw_set.insert("text");

	// create machete
	auto machete = std::make_shared<Dx9::Object>(meshes["machete"]);
	machete->sx = 0.005f;
	machete->sy = 0.005f;
	machete->sz = 0.005f;
	machete->x = 8;
	machete->y = 4;
	objs["machete"] = machete;
	draw_order["machete"] = 0;
	draw_set.insert("machete");

	// create machete bounding box and bounding sphere
	D3DXVECTOR3 min, max, center;
	float radius;
	if (!meshes["machete"]->ComputeBoundingBox(min, max))
		return 1;
	if (!meshes["machete"]->ComputeBoundingSphere(center, radius))
		return 1;
	cout << "machete bounding box : [(" << min.x << ", " << min.y << ", " << min.z << ")";
	cout << ", (" << max.x << ", " << max.y << ", " << max.z << ")]" << endl;
	cout << "machete bounding sphere : [(" << center.x << ", " << center.y << ", " << center.z << ")";
	cout << ", " << radius << "]" << endl;
	auto machete_box = std::make_shared<Dx9::Object>(meshes["machete_box"]);
	machete_box->mat.Diffuse.a = 0.5f;
	machete_box->sx = machete->sx * (max.x - min.x);
	machete_box->sy = machete->sy * (max.y - min.y);
	machete_box->sz = machete->sz * (max.z - min.z);
	machete_box->x = machete->x + (max.x + min.x) * 0.5f * machete->sx;
	machete_box->y = machete->y + (max.y + min.y) * 0.5f * machete->sy;
	machete_box->z = machete->z + (max.z + min.z) * 0.5f * machete->sz;
	objs["machete_box"] = machete_box;
	draw_order["machete_box"] = 30;
	draw_set.insert("machete_box");
	auto machete_sphere = std::make_shared<Dx9::Object>(meshes["machete_sphere"]);
	machete_sphere->mat.Diffuse.a = 0.5f;
	machete_sphere->sx = machete->sx * radius;
	machete_sphere->sy = machete->sy * radius;
	machete_sphere->sz = machete->sz * radius;
	machete_sphere->x = machete->x + center.x * machete->sx;
	machete_sphere->y = machete->y + center.y * machete->sy;
	machete_sphere->z = machete->z + center.z * machete->sz;
	objs["machete_sphere"] = machete_sphere;
	draw_order["machete_sphere"] = 30;

	// create terrain
	auto terrain = std::make_shared<Dx9::Object>(meshes["terrain"]);
	terrain->y = -18;
	objs["terrain"] = terrain;
	draw_order["terrain"] = 0;
	draw_set.insert("terrain");

	return 0;
}
int dx9_example_4_create_other(IDirect3DDevice9* device, std::map<std::string, std::shared_ptr<Dx9::Particles>>& particles,
	std::shared_ptr<Dx9::Camera>& camera,
	D3DLIGHT9& light,
	std::map<std::string, std::shared_ptr<Dx9::Texture>>& textures,
	SimpleWindowWithFC& window)
{
	HRESULT hr = 0;

	// create particles
	auto snow = std::make_shared<Dx9::SnowParticles>(device);
	if (!snow->IsAlive())
		return 1;
	snow->texture = textures["snow"];
	snow->y = -18;
	snow->range_min.x = -20;
	snow->range_max.x = 20;
	snow->range_min.z = -20;
	snow->range_max.z = 20;
	snow->range_min.y = -5;
	snow->range_max.y = 8;
	snow->emit_rate = 3000;
	particles["snow"] = snow;

	// create camera
	camera = std::make_shared<Dx9::Camera>();
	camera->aspect = static_cast<float>(window.GetWidth()) / window.GetHeight();
	camera->pos.z = 8;

	// create light
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

	return 0;
}
int dx9_example_adjust_state(IDirect3DDevice9* device, std::shared_ptr<Dx9::Camera>& camera,
	std::map<std::string, std::shared_ptr<Dx9::Object>>& objs,
	std::set<std::string>& draw_set,
	std::map<std::string, std::shared_ptr<Dx9::Particles >>& particles)
{
	HRESULT hr = 0;

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
	static bool init_mouse_pos = false;
	static POINT mouse_pos_old;
	if (!init_mouse_pos)
	{
		GetCursorPos(&mouse_pos_old);
		init_mouse_pos = true;
	}
	if (KeyIsDown(VK_MBUTTON))
	{
		POINT mouse_pos_new;
		GetCursorPos(&mouse_pos_new);
		camera->YawRight(0.003f * (mouse_pos_new.x - mouse_pos_old.x));
		camera->PitchDown(0.003f * (mouse_pos_new.y - mouse_pos_old.y));
		SetCursorPos(mouse_pos_old.x, mouse_pos_old.y);
	}
	else
		GetCursorPos(&mouse_pos_old);

	// control obj
	static auto control_obj = objs["text"];
	if (KeyIsDown(VK_NUMPAD1)) control_obj = objs["mirror"];
	else if (KeyIsDown(VK_NUMPAD2)) control_obj = objs["ground"];
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
	if (!objs["teapot"]->mesh->AdjustProgress(teapot_progress))
		cout << "failed : adjust teapot progress" << endl;

	// adjust show machete bounding
	if (KeyIsDown(VK_F3)) draw_set.erase("machete_box"), draw_set.erase("machete_sphere");
	else if (KeyIsDown(VK_F4)) draw_set.insert("machete_box"), draw_set.erase("machete_sphere");
	else if (KeyIsDown(VK_F5)) draw_set.erase("machete_box"), draw_set.insert("machete_sphere");

	// show information
	if (KeyIsDown(VK_F6)) cout << "snow particles count : " << particles["snow"]->GetParticlesCount() << endl;

	return 0;
}
int dx9_example()
{
	HRESULT hr = 0;

	SimpleWindowWithFC window(GetModuleHandle(nullptr), 800, 600, "dx9");
	if (!window.CheckWindowState())
		return 1;
	cout << "finish create window" << endl;

	IDirect3DDevice9* device = Dx9::CreateSimpleDx9Device(&window);
	if (!device)
		return 1;
	cout << "finish create dx9 device" << endl;

	// 1 create mesh
	std::map<std::string, std::shared_ptr<Dx9::Mesh>> meshes;
	if (dx9_example_1_create_mesh(device, meshes) != 0)
		return 1;

	// 2 create texture
	std::map<std::string, std::shared_ptr<Dx9::Texture>> textures;
	if (dx9_example_2_create_texture(device, textures) != 0)
		return 1;

	// 3 create object
	std::map<std::string, std::shared_ptr<Dx9::Object>> objs;
	std::map<std::string, int> draw_order;
	std::set<std::string> draw_set, draw_mirror_set, draw_shadow_set;
	if (dx9_example_3_create_object(objs, draw_order, draw_set, draw_mirror_set, draw_shadow_set, meshes, textures) != 0)
		return 1;

	// 4 create other
	std::map<std::string, std::shared_ptr<Dx9::Particles>> particles;
	std::shared_ptr<Dx9::Camera> camera;
	D3DLIGHT9 light;
	if (dx9_example_4_create_other(device, particles, camera, light, textures, window) != 0)
		return 1;

	// 5 set rendering state
	if (!Dx9::SetDrawMode(device, Dx9::DrawMode::Init))
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
		
		// input
		if (dx9_example_adjust_state(device, camera, objs, draw_set, particles) != 0)
			return 1;

		// update particles
		unsigned int dt = min(window.fps_counter.GetLastDeltatime(), 1000);
		for (auto& p : particles)
		{
			if (!p.second->Present(dt))
				return 1;
		}


		// begin
		hr = device->BeginScene();
		if (FAILED(hr))
			return 1;

		// 1 draw
		if (!Dx9::SetDrawMode(device, Dx9::DrawMode::Normal))
			return false;
		if (!camera->Transform(device))
			return 1;
		std::multimap<int, std::shared_ptr<Dx9::Object>> draw_sort;
		for (auto& id : draw_set)
			draw_sort.insert(std::make_pair(draw_order[id], objs[id]));
		for (auto& p : draw_sort)
		{
			if (!p.second->Draw(device))
				return 1;
		}

		// 2 draw particles
		if (!Dx9::SetDrawMode(device, Dx9::DrawMode::Particles))
			return false;
		if (!camera->Transform(device))
			return 1;
		for (auto& p : particles)
		{
			if (!p.second->Draw(device))
				return 1;
		}

		// 3-1 draw effect zone
		if (!Dx9::SetDrawMode(device, Dx9::DrawMode::Stencil))
			return false;
		if (!objs["ground"]->Draw(device))
			return 1;

		// compute ground plane
		auto ground = objs["ground"];
		D3DXVECTOR4 g_n0(0, 0, 1, 0), g_n1;
		D3DXMATRIX g_transform = ground->ComputeTransform();
		D3DXVec4Transform(&g_n1, &g_n0, &g_transform);
		D3DXVECTOR3 g_n2(g_n1), g_n3;
		D3DXVec3Normalize(&g_n3, &g_n2);
		D3DXPLANE plane_ground(g_n3.x, g_n3.y, g_n3.z, -(ground->x * g_n3.x + ground->y * g_n3.y + ground->z * g_n3.z));
		// compute light direction
		D3DXVECTOR4 light_dir = D3DXVECTOR4(light.Direction, 0);
		// 3-2 draw shadow
		if (!Dx9::SetDrawMode(device, Dx9::DrawMode::Shadow))
			return false;
		if (!camera->Transform(device))
			return 1;
		draw_sort.clear();
		for (auto& id : draw_shadow_set)
			draw_sort.insert(std::make_pair(draw_order[id], objs[id]));
		for (auto& p : draw_sort)
		{
			if (!p.second->DrawShadow(device, light_dir, plane_ground))
				return 1;
		}

		// 4-1 draw effect zone
		if (!Dx9::SetDrawMode(device, Dx9::DrawMode::Stencil))
			return false;
		if (!objs["mirror"]->Draw(device))
			return 1;

		// clear zbuffer
		hr = device->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		if (FAILED(hr))
			return 1;
		// compute mirror plane
		auto mirror = objs["mirror"];
		D3DXVECTOR4 n0(0, 0, 1, 0), n1;
		D3DXMATRIX transform = mirror->ComputeTransform();
		D3DXVec4Transform(&n1, &n0, &transform);
		D3DXVECTOR3 n2(n1), n3;
		D3DXVec3Normalize(&n3, &n2);
		D3DXPLANE plane_mirror(n3.x, n3.y, n3.z, -(mirror->x * n3.x + mirror->y * n3.y + mirror->z * n3.z));
		// 4-2 draw mirror
		if (!Dx9::SetDrawMode(device, Dx9::DrawMode::Mirror))
			return false;
		if (!camera->TransformReflect(device, plane_mirror))
			return 1;
		draw_sort.clear();
		for (auto& id : draw_mirror_set)
			draw_sort.insert(std::make_pair(draw_order[id], objs[id]));
		for (auto& p : draw_sort)
		{
			if (!p.second->Draw(device))
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