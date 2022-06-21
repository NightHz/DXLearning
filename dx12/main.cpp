#include <iostream>
#include "dx12.h"
#include "Rehenz/input.h"
#include <unordered_map>

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
	shader_lib["vs_light"] = UtilDx12::CompileShaderFile(L"dx12_vs_light.hlsl", "vs");
	shader_lib["ps_color"] = UtilDx12::CompileShaderFile(L"dx12_ps_color.hlsl", "ps");
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
	pso_lib["rehenz_pso"] = pso_creator.CreatePSO(device->device.Get());
	for (auto& p : pso_lib)
	{
		if (!p.second)
			return false;
	}

	// init meshs
	mesh_lib["cube"] = MeshDx12::CreateCube();
	mesh_lib["cube2"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateCubeMeshColorful(100));
	mesh_lib["sphere"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateSphereMesh(50));
	mesh_lib["sphere2"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateSphereMeshD(20));
	mesh_lib["cone"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateFrustumMesh(0, 50));
	mesh_lib["frustum"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateFrustumMesh(0.5f, 50));
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

	// init objs
	UINT cb_slot = 0;
	auto cube = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["cube2"]);
	cube->transform.pos = Rehenz::Vector(0, -2.5f, 0);
	cube->transform.scale = Rehenz::Vector(1.2f, 0.5f, 1.2f);
	cube->material = MaterialDx12::green;
	obj_lib["cube"] = cube;
	pso_objs["rehenz_pso"].push_back("cube");
	auto ground = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["cube2"]);
	ground->transform.pos = Rehenz::Vector(0, -4, 0);
	ground->transform.scale = Rehenz::Vector(10, 1, 10);
	ground->material = XMFLOAT4(0.2f, 0.2f, 0.2f, 1);
	obj_lib["ground"] = ground;
	pso_objs["rehenz_pso"].push_back("ground");
	for (float z = -6; z <= 6; z += 3)
	{
		for (float x = -6; x <= 6; x += 12)
		{
			std::string id = std::to_string(z) + (x < 6 ? "Left" : "Right");
			auto pillar = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["frustum"]);
			pillar->transform.pos = Rehenz::Vector(x, -1, z);
			pillar->transform.scale = Rehenz::Vector(0.8f, 2, 0.8f);
			pillar->material = MaterialDx12::yellow;
			obj_lib["pillar" + id] = pillar;
			pso_objs["rehenz_pso"].push_back("pillar" + id);
			auto sphere = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["sphere"]);
			sphere->transform.pos = Rehenz::Vector(x, 1.8f, z);
			sphere->transform.scale = Rehenz::Vector(0.8f, 0.8f, 0.8f);
			sphere->material = MaterialDx12::orange;
			obj_lib["sphere" + id] = sphere;
			pso_objs["rehenz_pso"].push_back("sphere" + id);
		}
	}
	auto point_light = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["sphere2"]);
	point_light->transform.pos = Rehenz::Vector(3, -2, 0);
	point_light->transform.scale = Rehenz::Vector(0.13f, 0.13f, 0.13f);
	point_light->material = MaterialDx12::black;
	point_light->material.emissive = MaterialDx12::white;
	obj_lib["point_light"] = point_light;
	pso_objs["rehenz_pso"].push_back("point_light");

	// init camera
	camera_trans.pos = Rehenz::Vector(1.2f, 1.6f, -4, 0);
	camera_trans.axes = Rehenz::AircraftAxes(0.4f, -0.3f, 0);
	camera_proj.z_far = 100;
	camera_proj.aspect = device->vp.Width / device->vp.Height;

	// init light
	cb_light.dl_enable = true;
	cb_light.dl_specular_enable = true;
	cb_light.dl_dir = XMFLOAT4(0, -1, 0.15f, 0);
	cb_light.dl_ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1);
	cb_light.dl_diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1);
	cb_light.dl_specular = XMFLOAT4(1, 1, 1, 1);
	cb_light.pl_enable = true;
	cb_light.pl_specular_enable = true;
	cb_light.pl_range = 3;
	cb_light.pl_pos = XMFLOAT4(point_light->transform.pos.x, point_light->transform.pos.y, point_light->transform.pos.z, 1);
	cb_light.pl_ambient = XMFLOAT4(0, 0, 0, 1);
	cb_light.pl_diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1);
	cb_light.pl_specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1);

	return true;
}

bool clean_after_init()
{
	for (auto& p : mesh_lib)
		p.second->FreeUploader();

	return true;
}

bool draw(DeviceDx12* device)
{
	auto& frc = device->GetCurrentFrameResource();

	// set cbuffer

	if (!frc.cb_frame->CopyData(0, cb_frame))
		return false;
	if (!frc.cb_light->CopyData(0, cb_light))
		return false;
	frc.SetRootParameterFrame(device->cmd_list.Get());

	// set pso and draw objects

	for (auto& pair : pso_objs)
	{
		device->cmd_list->SetPipelineState(pso_lib[pair.first].Get());
		for (auto& obj_name : pair.second)
		{
			if (!obj_lib[obj_name]->Draw(device->cmd_list.Get(), &frc))
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
		if (!device->ReadyPresent())
			return 1;
		if (!draw(device.get()))
			return 1;
		if (!device->Present())
			return 1;
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
