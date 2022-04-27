#include <iostream>
#include "Rehenz/window.h"
#include "input.h"
#include <unordered_map>
#include "dx11.h"

using std::cout;
using std::wcout;
using std::endl;
using namespace Dx11;

template <typename T>
using library = std::unordered_map<std::string, std::shared_ptr<T>>;

library<Mesh> meshes;
library<Texture> textures;
library<VertexShader> vses;
library<PixelShader> pses;
library<CBuffer> cbuffers;
library<Object> objs;
library<Camera> cams;
std::unordered_map<std::string, float> control_value;

int dx11_setup(Rehenz::SimpleWindow* window, Infrastructure* infra)
{
	HRESULT hr = 0;

	// list adapters
	auto adapter_descs = GetAdapterDescs();
	cout << "adapters : " << endl;
	for (auto& s : adapter_descs)
		wcout << L"  " << s << endl;

	// meshes
	meshes["triangle_xyz"] = Mesh::CreateTriangleXYZ(infra->device.Get());
	meshes["cube_color"] = Mesh::CreateCubeColor(infra->device.Get());
	meshes["cube"] = Mesh::CreateFromRehenzMesh(infra->device.Get(), Rehenz::CreateCubeMesh());
	meshes["sphere"] = Mesh::CreateFromRehenzMesh(infra->device.Get(), Rehenz::CreateSphereMesh());
	meshes["sphere_b"] = Mesh::CreateFromRehenzMesh(infra->device.Get(), Rehenz::CreateSphereMeshB());
	meshes["sphere_c"] = Mesh::CreateFromRehenzMesh(infra->device.Get(), Rehenz::CreateSphereMeshC());
	meshes["sphere_d"] = Mesh::CreateFromRehenzMesh(infra->device.Get(), Rehenz::CreateSphereMeshD());
	for (auto& p : meshes)
	{
		if (p.second == nullptr)
			return 10;
	}

	// textures
	textures["plaid"] = Texture::CreateTexturePlaid(infra->device.Get());
	for (auto& p : textures)
	{
		if (p.second == nullptr)
			return 11;
	}

	// vses
	vses["vs0"] = VertexShader::CompileVS(infra->device.Get(), L"vs0.hlsl");
	vses["vs_transform"] = VertexShader::CompileVS(infra->device.Get(), L"vs_transform.hlsl");
	vses["vs_light"] = VertexShader::CompileVS(infra->device.Get(), L"vs_light.hlsl");
	for (auto& p : vses)
	{
		if (p.second == nullptr)
			return 20;
	}

	// pses
	pses["ps0"] = PixelShader::CompilePS(infra->device.Get(), L"ps0.hlsl");
	pses["ps_color"] = PixelShader::CompilePS(infra->device.Get(), L"ps_color.hlsl");
	pses["ps_tex"] = PixelShader::CompilePS(infra->device.Get(), L"ps_tex.hlsl");
	for (auto& p : pses)
	{
		if (p.second == nullptr)
			return 21;
	}

	// cbuffers
	auto vscb_transform = CBuffer::CreateCBuffer(infra->device.Get(), sizeof(VSCBTransform));
	cbuffers["transform"] = vscb_transform;
	auto vscb_material = CBuffer::CreateCBuffer(infra->device.Get(), sizeof(VSCBMaterial));
	cbuffers["material"] = vscb_material;
	auto vscb_light = CBuffer::CreateCBuffer(infra->device.Get(), sizeof(VSCBLight));
	VSCBLight* vscb_light_struct = static_cast<VSCBLight*>(vscb_light->GetPointer());
	vscb_light_struct->dl_enable = true;
	vscb_light_struct->dl_specular_enable = true;
	vscb_light_struct->dl_dir = DirectX::XMVectorSet(0, -1, -0.15f, 0);
	vscb_light_struct->dl_ambient = DirectX::XMVectorSet(0.2f, 0.2f, 0.2f, 1);
	vscb_light_struct->dl_diffuse = DirectX::XMVectorSet(1, 1, 1, 1);
	vscb_light_struct->dl_specular = DirectX::XMVectorSet(1, 1, 1, 1);
	vscb_light_struct->pl_enable = true;
	vscb_light_struct->pl_specular_enable = true;
	vscb_light_struct->pl_range = 2;
	vscb_light_struct->pl_pos = DirectX::XMVectorSet(-3, -3, 0, 1);
	vscb_light_struct->pl_ambient = DirectX::XMVectorSet(0, 0, 0, 1);
	vscb_light_struct->pl_diffuse = DirectX::XMVectorSet(0.5f, 1, 0.5f, 1);
	vscb_light_struct->pl_specular = DirectX::XMVectorSet(0.5f, 1, 0.5f, 1);
	assert(vscb_light->ApplyToCBuffer(infra->context.Get()) == true);
	infra->context->VSSetConstantBuffers(vscb_light_struct->slot, 1, vscb_light->GetBuffer().GetAddressOf());
	cbuffers["light"] = vscb_light;
	for (auto& p : cbuffers)
	{
		if (p.second == nullptr)
			return 30;
	}

	// some state interface
	ComPtr<ID3D11RasterizerState> rs_nocull, rs_frontcull;
	ComPtr<ID3D11BlendState> bs_alpha;
	D3D11_RASTERIZER_DESC rd;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE;
	rd.FrontCounterClockwise = false;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0;
	rd.SlopeScaledDepthBias = 0;
	rd.DepthClipEnable = true;
	rd.ScissorEnable = false;
	rd.MultisampleEnable = false;
	rd.AntialiasedLineEnable = false;
	hr = infra->device->CreateRasterizerState(&rd, rs_nocull.GetAddressOf());
	if (FAILED(hr))
		return 31;
	rd.CullMode = D3D11_CULL_FRONT;
	hr = infra->device->CreateRasterizerState(&rd, rs_frontcull.GetAddressOf());
	if (FAILED(hr))
		return 31;
	D3D11_BLEND_DESC bd;
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = infra->device->CreateBlendState(&bd, bs_alpha.GetAddressOf());
	if (FAILED(hr))
		return 31;

	// objs
	auto triangle = std::make_shared<Object>(infra->device.Get(), meshes["triangle_xyz"], vses["vs0"], pses["ps0"], nullptr, nullptr);
	objs["triangle"] = triangle;
	auto cube_color = std::make_shared<Object>(infra->device.Get(), meshes["cube_color"], vses["vs_transform"], pses["ps_color"], vscb_transform, vscb_material);
	cube_color->transform.roll = std::atanf(1);
	cube_color->transform.pitch = std::atanf(std::sqrtf(0.5f));
	cube_color->transform.yaw = -2.7f;
	objs["cube_color"] = cube_color;
	auto cube = std::make_shared<Object>(infra->device.Get(), meshes["cube"], vses["vs_light"], pses["ps_color"], vscb_transform, vscb_material);
	cube->transform.roll = std::atanf(1);
	cube->transform.pitch = std::atanf(std::sqrtf(0.5f));
	cube->transform.yaw = -2.7f;
	cube->transform.pos.x = -3;
	cube->material = Material::orange;
	objs["cube"] = cube;
	auto sphere = std::make_shared<Object>(infra->device.Get(), meshes["sphere"], vses["vs_light"], pses["ps_color"], vscb_transform, vscb_material);
	sphere->transform.pos.y = -3;
	sphere->material = Material::orange;
	objs["sphere"] = sphere;
	auto sphere_b = std::make_shared<Object>(infra->device.Get(), meshes["sphere_b"], vses["vs_light"], pses["ps_color"], vscb_transform, vscb_material);
	sphere_b->transform.pos.x = -6;
	sphere_b->transform.pos.y = -3;
	sphere_b->transform.SetScale(0.5f);
	sphere_b->material = Material::orange;
	objs["sphere_b"] = sphere_b;
	auto sphere_d = std::make_shared<Object>(infra->device.Get(), meshes["sphere_d"], vses["vs_light"], pses["ps_color"], vscb_transform, vscb_material);
	sphere_d->transform.pos.x = -4.5f;
	sphere_d->transform.pos.y = -1.5f;
	sphere_d->transform.SetScale(0.5f);
	sphere_d->material = Material::orange;
	objs["sphere_d"] = sphere_d;
	auto light = std::make_shared<Object>(infra->device.Get(), meshes["sphere_c"], vses["vs_light"], pses["ps_color"], vscb_transform, vscb_material);
	DirectX::XMStoreFloat3(&light->transform.pos, vscb_light_struct->pl_pos);
	light->transform.SetScale(0.2f);
	light->material = Material::black;
	light->material.emissive = DirectX::XMFLOAT4(1, 1, 1, 1);
	objs["light"] = light;
	auto cube_tex = std::make_shared<Object>(infra->device.Get(), meshes["cube"], vses["vs_light"], pses["ps_tex"], vscb_transform, vscb_material);
	cube_tex->transform.roll = std::atanf(1);
	cube_tex->transform.pitch = std::atanf(std::sqrtf(0.5f));
	cube_tex->transform.yaw = -2.7f;
	cube_tex->transform.pos.x = 3;
	cube_tex->material = Material::white;
	cube_tex->textures.push_back(textures["plaid"]);
	objs["cube_tex"] = cube_tex;
	auto sphere_alpha = std::make_shared<Object>(infra->device.Get(), meshes["sphere"], vses["vs_light"], pses["ps_color"], vscb_transform, vscb_material);
	sphere_alpha->transform.pos.x = 3;
	sphere_alpha->transform.pos.z = -2;
	sphere_alpha->transform.SetScale(0.5f);
	sphere_alpha->material = DirectX::XMFLOAT4(1, 1, 1, 0.5f);
	//sphere_alpha->rs = rs_nocull;
	sphere_alpha->bs = bs_alpha;
	objs["sphere_alpha"] = sphere_alpha;
	for (auto& p : objs)
	{
		if (!*p.second)
			return 40;
	}

	// cams
	auto dsv_buffer = Texture::CreateTextureForDSV(infra->device.Get(), window->GetWidth(), window->GetHeight());
	auto cam = std::make_shared<Camera>(infra->device.Get(), infra->sc.Get(), dsv_buffer.Get(),
		vscb_transform, static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));
	cam->transform.pos.z = -5;
	cams["cam"] = cam;
	for (auto& p : cams)
	{
		if (!*p.second)
			return 50;
	}

	// control_value
	control_value["bg_r"] = 0.7804f;
	control_value["bg_g"] = 0.8627f;
	control_value["bg_b"] = 0.4078f;

	return 0;
}

int dx11_control(Infrastructure* infra, float dt)
{
	HRESULT hr = 0;
	static bool first = true;

	// bg color
	if (KeyIsDown('1')) control_value["bg_r"] = 0.0784f;
	else control_value["bg_r"] = 0.7804f;
	if (KeyIsDown('2')) control_value["bg_g"] = 0.0784f;
	else control_value["bg_g"] = 0.8627f;
	if (KeyIsDown('3')) control_value["bg_b"] = 0.0784f;
	else control_value["bg_b"] = 0.4078f;

	// control light
	auto vscb_light = cbuffers["light"];
	VSCBLight* vscb_light_struct = static_cast<VSCBLight*>(vscb_light->GetPointer());
	if (KeyIsDown('4')) vscb_light_struct->dl_enable = true;
	else if (KeyIsDown('5')) vscb_light_struct->dl_enable = false;
	if (KeyIsDown('6')) { vscb_light_struct->pl_enable = true; objs["light"]->material.emissive = DirectX::XMFLOAT4(1, 1, 1, 1); }
	else if (KeyIsDown('7')) { vscb_light_struct->pl_enable = false; objs["light"]->material.emissive = DirectX::XMFLOAT4(0, 0, 0, 1); }
	assert(vscb_light->ApplyToCBuffer(infra->context.Get()) == true);
	infra->context->VSSetConstantBuffers(vscb_light_struct->slot, 1, vscb_light->GetBuffer().GetAddressOf());

	// control obj
	static auto control_obj = objs["cube_tex"];
	float obj_rotate_angle = 3 * dt;
	if (KeyIsDown('I')) control_obj->transform.pitch += obj_rotate_angle;
	else if (KeyIsDown('K')) control_obj->transform.pitch -= obj_rotate_angle;
	if (KeyIsDown('J')) control_obj->transform.yaw += obj_rotate_angle;
	else if (KeyIsDown('L')) control_obj->transform.yaw -= obj_rotate_angle;
	if (KeyIsDown('U')) control_obj->transform.roll -= obj_rotate_angle;
	else if (KeyIsDown('O')) control_obj->transform.roll += obj_rotate_angle;

	// control camera
	static auto control_cam = cams["cam"];
	float cam_move_dis = 5 * dt;
	float cam_rotate_angle = 0.3f * dt;
	static POINT mouse_pos; if (first) GetCursorPos(&mouse_pos);
	if (KeyIsDown('W')) control_cam->transform.PosAddOffset(control_cam->transform.GetFrontXZ(), cam_move_dis);
	else if (KeyIsDown('S')) control_cam->transform.PosAddOffset(control_cam->transform.GetFrontXZ(), -cam_move_dis);
	if (KeyIsDown('A')) control_cam->transform.PosAddOffset(control_cam->transform.GetRightXZ(), -cam_move_dis);
	else if (KeyIsDown('D')) control_cam->transform.PosAddOffset(control_cam->transform.GetRightXZ(), cam_move_dis);
	if (KeyIsDown(VK_SPACE)) control_cam->transform.pos.y += cam_move_dis;
	else if (KeyIsDown(VK_LSHIFT)) control_cam->transform.pos.y -= cam_move_dis;
	if (KeyIsDown(VK_MBUTTON))
	{
		POINT mouse_pos2;
		GetCursorPos(&mouse_pos2);
		control_cam->transform.pitch += cam_rotate_angle * (mouse_pos2.y - mouse_pos.y);
		control_cam->transform.yaw += cam_rotate_angle * (mouse_pos2.x - mouse_pos.x);
		SetCursorPos(mouse_pos.x, mouse_pos.y);
	}
	else
		GetCursorPos(&mouse_pos);

	first = false;
	return 0;
}


int dx11_render(Infrastructure* infra)
{
	HRESULT hr = 0;

	// clear and set camera
	auto& cam = cams["cam"];
	cam->Clear(infra->context.Get(), control_value["bg_r"], control_value["bg_g"], control_value["bg_b"], 1);
	cam->SetToContext(infra->context.Get());

	// draw
	//objs["triangle"]->Draw(infra->context.Get());
	objs["cube_color"]->Draw(infra->context.Get());
	objs["cube"]->Draw(infra->context.Get());
	objs["sphere"]->Draw(infra->context.Get());
	objs["sphere_b"]->Draw(infra->context.Get());
	objs["sphere_d"]->Draw(infra->context.Get());
	objs["light"]->Draw(infra->context.Get());
	objs["cube_tex"]->Draw(infra->context.Get());
	objs["sphere_alpha"]->Draw(infra->context.Get());

	// present
	hr = infra->sc->Present(0, 0);
	if (FAILED(hr))
		return 1;

	return 0;
}

int dx11_example()
{
	int r;
	wcout.imbue(std::locale("", LC_CTYPE)); // set to system code

	auto window = std::make_shared<Rehenz::SimpleWindowWithFC>(GetModuleHandle(nullptr), 800, 600, "dx11");
	if (!window->CheckWindowState())
		return 1;
	cout << "finish create window" << endl;

	auto infra = CreateSimpleD3d11Device(window.get());
	if (infra == nullptr)
		return 1;
	cout << "finish create dx11 device" << endl;
	
	r = dx11_setup(window.get(), infra.get());
	if (r != 0)
		return r;
	cout << "finish setup" << endl;

	while (true)
	{
		// control
		r = dx11_control(infra.get(), window->fps_counter.GetLastDeltatime() / 1000.0f);
		if (r != 0)
			return r;

		// render
		r = dx11_render(infra.get());
		if (r != 0)
			return r;

		// present
		window->Present();

		// msg
		Rehenz::SimpleMessageProcess();
		if (!window->CheckWindowState() || KeyIsDown('Q'))
			break;
	}

	// release
	cout << "finish release" << endl;

	return 0;
}
