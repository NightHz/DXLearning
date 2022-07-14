#include <iostream>
#include "dx12.h"
#include "Rehenz/input.h"
#include <unordered_map>
#include <timeapi.h>

using std::cout;
using std::wcout;
using std::endl;
using namespace Dx12;
using Rehenz::KeyIsDown;

Rehenz::Mouse mouse;

template <typename T>
using base_library = std::unordered_map<std::string, T>;
base_library<ComPtr<ID3D12PipelineState>> pso_lib;
base_library<std::shared_ptr<std::vector<D3D12_INPUT_ELEMENT_DESC>>> il_lib;
base_library<ComPtr<ID3DBlob>> shader_lib;
base_library<std::shared_ptr<MeshDx12>> mesh_lib;
base_library<std::shared_ptr<MaterialDx12>> mat_lib;
base_library<std::shared_ptr<TextureDx12>> tex_lib;
base_library<UINT> sampler_slots;
base_library<std::shared_ptr<ObjectDx12>> obj_lib;

Rehenz::Transform camera_trans;
Rehenz::Projection camera_proj;

CBFrame cb_frame;
CBLight cb_light;

base_library<std::vector<std::string>> pso_objs; // pso - objs


void update(DeviceDx12* device, float dt)
{
	static float t = 0;
	t += dt;

	// control camera
	float cam_move_dis = 5 * dt;
	float cam_rotate_angle = 0.005f;
	if (KeyIsDown('W')) camera_trans.pos += camera_trans.GetFrontInGround() * cam_move_dis;
	else if (KeyIsDown('S')) camera_trans.pos -= camera_trans.GetFrontInGround() * cam_move_dis;
	if (KeyIsDown('A')) camera_trans.pos -= camera_trans.GetRightInGround() * cam_move_dis;
	else if (KeyIsDown('D')) camera_trans.pos += camera_trans.GetRightInGround() * cam_move_dis;
	if (KeyIsDown(VK_SPACE)) camera_trans.pos.y += cam_move_dis;
	else if (KeyIsDown(VK_LSHIFT)) camera_trans.pos.y -= cam_move_dis;
	if (KeyIsDown(VK_MBUTTON))
	{
		camera_trans.axes.pitch += cam_rotate_angle * mouse.GetMoveY();
		camera_trans.axes.yaw += cam_rotate_angle * mouse.GetMoveX();
		mouse.SetToPrev();
	}
	if (KeyIsDown('R'))
	{
		camera_trans.pos = Rehenz::Vector(-4, 3.5f, -10, 0);
		camera_trans.axes = Rehenz::AircraftAxes(0.44f, 0.4f, 0);
	}

	// control object
	float obj_rotate_angle = 3 * dt;
	auto obj = obj_lib["cube"];
	if (KeyIsDown('I')) obj->transform.axes.pitch += obj_rotate_angle;
	else if (KeyIsDown('K')) obj->transform.axes.pitch -= obj_rotate_angle;
	if (KeyIsDown('J')) obj->transform.axes.yaw -= obj_rotate_angle;
	else if (KeyIsDown('L')) obj->transform.axes.yaw += obj_rotate_angle;
	if (KeyIsDown('U')) obj->transform.axes.roll -= obj_rotate_angle;
	else if (KeyIsDown('O')) obj->transform.axes.roll += obj_rotate_angle;

	// control light
	if (KeyIsDown('1')) cb_light.lights[0].type = light_type_directional;
	else if (KeyIsDown('2')) cb_light.lights[0].type = 0;
	if (KeyIsDown('3')) cb_light.lights[1].type = light_type_point;
	else if (KeyIsDown('4')) cb_light.lights[1].type = 0;
	if (KeyIsDown('5')) cb_light.lights[2].type = light_type_spot;
	else if (KeyIsDown('6')) cb_light.lights[2].type = 0;
	dxm::XMStoreFloat3(&cb_light.lights[2].direction, ToXmVector(camera_trans.GetFront()));
	dxm::XMStoreFloat3(&cb_light.lights[2].position, ToXmVector(camera_trans.pos - Rehenz::Vector(0, 0.5f, 0)));

	// update cbuffer struct
	XMMATRIX view = ToXmMatrix(camera_trans.GetInverseTransformMatrix());
	XMMATRIX inv_view = ToXmMatrix(camera_trans.GetTransformMatrix());
	XMMATRIX proj = ToXmMatrix(camera_proj.GetTransformMatrix());
	dxm::XMStoreFloat4x4(&cb_frame.view, dxm::XMMatrixTranspose(view));
	dxm::XMStoreFloat4x4(&cb_frame.inv_view, dxm::XMMatrixTranspose(inv_view));
	dxm::XMStoreFloat4x4(&cb_frame.proj, dxm::XMMatrixTranspose(proj));
	dxm::XMStoreFloat4x4(&cb_frame.view_proj, dxm::XMMatrixTranspose(view * proj));
	dxm::XMStoreFloat3(&cb_frame.eye_pos, ToXmVector(camera_trans.pos));
	dxm::XMStoreFloat3(&cb_frame.eye_at, ToXmVector(camera_trans.GetFront()));
	cb_frame.screen_size = XMFLOAT2(device->vp.Width, device->vp.Height);
	cb_frame.time = t;
	cb_frame.deltatime = dt;
}

bool init(DeviceDx12* device)
{
	// init input layout
	std::vector<D3D12_INPUT_ELEMENT_DESC> il;
	il.resize(2);
	il[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	il[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	il_lib["pos+color"] = std::make_shared<std::vector<D3D12_INPUT_ELEMENT_DESC>>(std::move(il));
	il.resize(5);
	il[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	il[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	il[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	il[3] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	il[4] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	il_lib["rehenz"] = std::make_shared<std::vector<D3D12_INPUT_ELEMENT_DESC>>(std::move(il));

	// init shader
	shader_lib["vs_transform"] = UtilDx12::CompileShaderFile(L"dx12_vs_transform.hlsl", "vs");
	shader_lib["vs_transform4"] = UtilDx12::CompileShaderFile(L"dx12_vs_transform4.hlsl", "vs");
	shader_lib["vs_light"] = UtilDx12::CompileShaderFile(L"dx12_vs_light.hlsl", "vs");
	shader_lib["ps_color"] = UtilDx12::CompileShaderFile(L"dx12_ps_color.hlsl", "ps");
	shader_lib["ps_light"] = UtilDx12::CompileShaderFile(L"dx12_ps_light.hlsl", "ps");
	shader_lib["ps_light_tex1"] = UtilDx12::CompileShaderFile(L"dx12_ps_light_tex1.hlsl", "ps");
	for (auto& p : shader_lib)
	{
		if (!p.second)
			return false;
	}

	// init pso
	PipelineStateCreatorDx12 pso_creator;
	pso_creator.SetRootSignature(device->root_sig.Get());
	pso_creator.SetInputLayout(*il_lib["pos+color"]);
	pso_creator.SetVS(shader_lib["vs_transform"].Get());
	pso_creator.SetPS(shader_lib["ps_color"].Get());
	//pso_creator.SetRSFillMode(D3D12_FILL_MODE_WIREFRAME);
	//pso_creator.SetRSCullMode(D3D12_CULL_MODE_NONE);
	pso_lib["default_pso"] = pso_creator.CreatePSO(device->device.Get());
	pso_creator.SetInputLayout(*il_lib["rehenz"]);
	pso_creator.SetVS(shader_lib["vs_light"].Get());
	pso_creator.SetPS(shader_lib["ps_color"].Get());
	pso_lib["vslight"] = pso_creator.CreatePSO(device->device.Get());
	pso_creator.SetInputLayout(*il_lib["rehenz"]);
	pso_creator.SetVS(shader_lib["vs_transform4"].Get());
	pso_creator.SetPS(shader_lib["ps_light"].Get());
	pso_lib["pslight"] = pso_creator.CreatePSO(device->device.Get());
	pso_creator.SetInputLayout(*il_lib["rehenz"]);
	pso_creator.SetVS(shader_lib["vs_transform4"].Get());
	pso_creator.SetPS(shader_lib["ps_light_tex1"].Get());
	pso_lib["pslight_tex1"] = pso_creator.CreatePSO(device->device.Get());
	for (auto& p : pso_lib)
	{
		if (!p.second)
			return false;
	}

	// init meshs
	mesh_lib["cube"] = MeshDx12::CreateCube();
	mesh_lib["cube2"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateCubeMeshColorful());
	mesh_lib["sphere"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateSphereMesh(50));
	mesh_lib["sphere2"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateSphereMeshD());
	mesh_lib["cone"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateFrustumMesh(0));
	mesh_lib["frustum"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateFrustumMesh());
	for (auto& p : mesh_lib)
	{
		if (!p.second)
			return false;
	}
	if (!mesh_lib["cube"]->UploadToGpu(device->device.Get(), device->cmd_list.Get()))
		return false;
	if (!MeshDx12::MergeUploadToGpu(
		std::vector<MeshDx12*>{ mesh_lib["cube2"].get(), mesh_lib["sphere"].get(), mesh_lib["sphere2"].get(), mesh_lib["cone"].get(), mesh_lib["frustum"].get() },
		device->device.Get(), device->cmd_list.Get()))
		return false;

	// init texs
	tex_lib["plaid"] = TextureDx12::CreateTexturePlaid();
	for (auto& p : tex_lib)
	{
		if (!p.second)
			return false;
	}
	for (auto& p : tex_lib)
	{
		if (!p.second->UploadToGpu(device->device.Get(), device->cmd_list.Get()))
			return false;
	}

	// init samplers
	D3D12_SAMPLER_DESC sampler_desc;
	sampler_slots["default"] = device->GetSamplerSlot(6);
	sampler_desc = UtilDx12::GetSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	device->device->CreateSampler(&sampler_desc, device->GetSampler(sampler_slots["default"] + 0));
	sampler_desc = UtilDx12::GetSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	device->device->CreateSampler(&sampler_desc, device->GetSampler(sampler_slots["default"] + 1));
	sampler_desc = UtilDx12::GetSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	device->device->CreateSampler(&sampler_desc, device->GetSampler(sampler_slots["default"] + 2));
	sampler_desc = UtilDx12::GetSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	device->device->CreateSampler(&sampler_desc, device->GetSampler(sampler_slots["default"] + 3));
	sampler_desc = UtilDx12::GetSamplerDesc(D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	device->device->CreateSampler(&sampler_desc, device->GetSampler(sampler_slots["default"] + 4));
	sampler_desc = UtilDx12::GetSamplerDesc(D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	device->device->CreateSampler(&sampler_desc, device->GetSampler(sampler_slots["default"] + 5));

	// init mats
	auto mat_grass = std::make_shared<MaterialDx12>();
	mat_grass->diffuse_albedo = XMFLOAT3(0.2f, 0.6f, 0.2f);
	mat_grass->alpha = 1.0f;
	mat_grass->fresnel_r0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	mat_grass->roughness = 0.125f;
	mat_lib["grass"] = mat_grass;
	auto mat_water = std::make_shared<MaterialDx12>();
	mat_water->diffuse_albedo = XMFLOAT3(0, 0.2f, 0.6f);
	mat_water->alpha = 1.0f;
	mat_water->fresnel_r0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat_water->roughness = 0;
	mat_lib["water"] = mat_water;
	auto mat_light = std::make_shared<MaterialDx12>();
	mat_light->diffuse_albedo = XMFLOAT3(0, 0, 0);
	mat_light->alpha = 1.0f;
	mat_light->fresnel_r0 = XMFLOAT3(0, 0, 0);
	mat_light->roughness = 0;
	mat_light->emissive = XMFLOAT3(1, 1, 1);
	mat_lib["light"] = mat_light;
	auto mat_orange = std::make_shared<MaterialDx12>();
	mat_orange->diffuse_albedo = XMFLOAT3(1, 0.5f, 0.14f);
	mat_orange->alpha = 1.0f;
	mat_orange->fresnel_r0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat_orange->roughness = 0.08f;
	mat_lib["orange"] = mat_orange;
	auto mat_yellow = std::make_shared<MaterialDx12>();
	mat_yellow->diffuse_albedo = XMFLOAT3(0.91f, 0.88f, 0.34f);
	mat_yellow->alpha = 1.0f;
	mat_yellow->fresnel_r0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mat_yellow->roughness = 0.08f;
	mat_lib["yellow"] = mat_yellow;
	auto mat_box = std::make_shared<MaterialDx12>();
	mat_box->tex_dh_slot = device->GetCbvSlot();
	tex_lib["plaid"]->CreateSrv(mat_box->tex_dh_slot, device);
	mat_box->diffuse_albedo = XMFLOAT3(1, 1, 1);
	mat_box->alpha = 1.0f;
	mat_box->fresnel_r0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	mat_box->roughness = 0.1f;
	mat_lib["box"] = mat_box;

	// init objs
	UINT cb_slot = 0;
	auto cube = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["cube2"], mat_lib["grass"]);
	cube->transform.pos = Rehenz::Vector(0, -2.7f, 0);
	cube->transform.scale = Rehenz::Vector(2.2f, 0.3f, 2.2f);
	obj_lib["cube"] = cube;
	pso_objs["pslight"].push_back("cube");
	auto ground = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["cube2"], mat_lib["water"]);
	ground->transform.pos = Rehenz::Vector(0, -4, 0);
	ground->transform.scale = Rehenz::Vector(10, 1, 10);
	obj_lib["ground"] = ground;
	pso_objs["pslight"].push_back("ground");
	for (float z = -6; z <= 6; z += 3)
	{
		for (float x = -6; x <= 6; x += 12)
		{
			std::string id = std::to_string(z) + (x < 6 ? "Left" : "Right");
			auto pillar = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["frustum"], mat_lib["yellow"]);
			pillar->transform.pos = Rehenz::Vector(x, -1, z);
			pillar->transform.scale = Rehenz::Vector(0.8f, 2, 0.8f);
			obj_lib["pillar" + id] = pillar;
			pso_objs["pslight"].push_back("pillar" + id);
			auto sphere = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["sphere"], mat_lib["orange"]);
			sphere->transform.pos = Rehenz::Vector(x, 1.8f, z);
			sphere->transform.scale = Rehenz::Vector(0.8f, 0.8f, 0.8f);
			obj_lib["sphere" + id] = sphere;
			pso_objs["pslight"].push_back("sphere" + id);
		}
	}
	auto point_light = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["sphere2"], mat_lib["light"]);
	point_light->transform.pos = Rehenz::Vector(3, -2, 0);
	point_light->transform.scale = Rehenz::Vector(0.13f, 0.13f, 0.13f);
	obj_lib["point_light"] = point_light;
	pso_objs["vslight"].push_back("point_light");
	auto box = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["cube2"], mat_lib["box"]);
	box->transform.pos = Rehenz::Vector(3.5f, -2.5f, 2.5f);
	box->transform.axes = Rehenz::AircraftAxes(0, -0.2f, 0);
	box->transform.scale = Rehenz::Vector(0.5f, 0.5f, 0.5f);
	obj_lib["box"] = box;
	pso_objs["pslight_tex1"].push_back("box");

	// init camera
	camera_trans.pos = Rehenz::Vector(1.2f, 1.6f, -4, 0);
	camera_trans.axes = Rehenz::AircraftAxes(0.4f, -0.3f, 0);
	camera_proj.z_far = 100;
	camera_proj.aspect = device->vp.Width / device->vp.Height;

	// init light
	::memset(&cb_light, 0, sizeof(cb_light));
	cb_light.light_enable = 1;
	cb_light.light_ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
	cb_light.lights[0].type = light_type_directional;
	cb_light.lights[0].intensity = XMFLOAT3(0.8f, 0.8f, 0.8f);
	cb_light.lights[0].direction = XMFLOAT3(0, -1, 0.15f);
	cb_light.lights[1].type = light_type_point;
	cb_light.lights[1].intensity = XMFLOAT3(0.5f, 0.5f, 0.5f);
	cb_light.lights[1].position = XMFLOAT3(point_light->transform.pos.x, point_light->transform.pos.y, point_light->transform.pos.z);
	cb_light.lights[1].falloff_begin = 3;
	cb_light.lights[1].falloff_end = 6;
	cb_light.lights[2].type = 0;
	cb_light.lights[2].intensity = XMFLOAT3(0.8f, 0.8f, 0.8f);
	dxm::XMStoreFloat3(&cb_light.lights[2].direction, ToXmVector(camera_trans.GetFront()));
	dxm::XMStoreFloat3(&cb_light.lights[2].position, ToXmVector(camera_trans.pos - Rehenz::Vector(0, 0.5f, 0)));
	cb_light.lights[2].falloff_begin = 5;
	cb_light.lights[2].falloff_end = 15;
	cb_light.lights[2].spot_divergence = 0.9f;

	return true;
}

bool clean_after_init()
{
	for (auto& p : mesh_lib)
		p.second->FreeUploader();
	for (auto& p : tex_lib)
		p.second->FreeUploader();

	return true;
}

bool draw(DeviceDx12* device)
{
	auto& frc = device->GetCurrentFrameResource();

	// set root parameters

	if (!frc.cb_frame->CopyData(0, cb_frame))
		return false;
	if (!frc.cb_light->CopyData(0, cb_light))
		return false;
	device->SetRootParameter1(frc.cb_frame_dh_slot);

	// set pso and draw objects

	for (auto& pair : pso_objs)
	{
		device->cmd_list->SetPipelineState(pso_lib[pair.first].Get());
		for (auto& obj_name : pair.second)
		{
			device->SetRootParameter3(sampler_slots["default"]);
			if (!obj_lib[obj_name]->Draw(device))
				return false;
		}
	}

	return true;
}

void clean()
{
	pso_lib.clear();
	il_lib.clear();
	shader_lib.clear();
	mesh_lib.clear();
	mat_lib.clear();
	tex_lib.clear();
	sampler_slots.clear();
	obj_lib.clear();
}

int main()
{
	cout << "dx12 learning ..." << endl;
	wcout.imbue(std::locale("", LC_CTYPE)); // set to system code

	// print adapter info
	DeviceDx12::PrintAdapterOutputInfo(wcout);

	// create window
	auto window = std::make_shared<Rehenz::SimpleWindowWithFC>(GetModuleHandle(nullptr), 800, 600, "dx12 example");
	if (!window->CheckWindowState())
		return 1;
	window->fps_counter.LockFps(0);
	cout << "finish create window" << endl;

	// create device
	auto device = DeviceDx12::CreateDevice(window.get());
	if (!device)
		return 1;
	cout << "finish create dx12 device" << endl;

	// print device info
	device->PrintSupportInfo(cout);

	// create second fps counter to count draw fps
	auto fps_counter2 = std::make_shared<Rehenz::FpsCounter>(timeGetTime);
	auto updateFps = [&window, &fps_counter2](Rehenz::uint fps)
	{
		window->SetTitle(window->title_base + TEXT(" fps:") + ToString(fps) + TEXT(" draw fps:") + ToString(fps_counter2->GetLastFps()));
	};
	window->fps_counter.UpdateFpsCallback = updateFps;

	// init
	if (!device->ResetCmd())
		return 1;
	if (!init(device.get()))
		return 1;
	if (!device->FinishCmd())
		return 1;

	// clean after init
	if (!device->FlushCmdQueue())
		return 1;
	if (!clean_after_init())
		return 1;

	// loop
	while (true)
	{
		// update
		mouse.Present();
		update(device.get(), window->fps_counter.GetLastDeltatime() / 1000.0f);

		// draw
		if (device->CheckCurrentCmdState())
		{
			if (!device->ReadyPresent())
				return 1;
			if (!draw(device.get()))
				return 1;
			if (!device->Present())
				return 1;
			fps_counter2->Present();
		}
		window->Present();

		// msg
		Rehenz::SimpleMessageProcess();
		// exit
		if (KeyIsDown('Q') || !window->CheckWindowState())
			break;
	}

	// clean
	if (!device->FlushCmdQueue())
		return 1;
	clean();
	device = nullptr;
	window = nullptr;

	return 0;
}
