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
		camera_trans.pos = Rehenz::Vector(1.2f, 1.6f, -4, 0);
		camera_trans.axes = Rehenz::AircraftAxes(0.4f, -0.3f, 0);
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
	XMMATRIX proj = ToXmMatrix(camera_proj.GetTransformMatrix());
	dxm::XMStoreFloat4x4(&cb_frame.view, dxm::XMMatrixTranspose(view));
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
	pso_lib["pso1"] = pso_creator.CreatePSO(device->device.Get());
	pso_creator.SetInputLayout(*il_lib["rehenz"]);
	pso_lib["pso2"] = pso_creator.CreatePSO(device->device.Get());
	for (auto& p : pso_lib)
	{
		if (!p.second)
			return false;
	}

	// init meshs
	mesh_lib["cube"] = MeshDx12::CreateCube();
	mesh_lib["cube2"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateCubeMeshColorful());
	mesh_lib["sphere"] = MeshDx12::CreateFromRehenzMesh(Rehenz::CreateSphereMesh());
	for (auto& p : mesh_lib)
	{
		if (!p.second)
			return false;
	}
	if (!mesh_lib["cube"]->UploadToGpu(device->device.Get(), device->cmd_list.Get()))
		return false;
	if (!MeshDx12::MergeUploadToGpu(std::vector<MeshDx12*>{ mesh_lib["cube2"].get(), mesh_lib["sphere"].get() }, device->device.Get(), device->cmd_list.Get()))
		return false;

	// init objs
	UINT cb_slot = 0;
	obj_lib["cube"] = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["cube"]);
	pso_objs["pso1"].push_back("cube");
	auto cube2 = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["cube2"]);
	cube2->transform.pos.x = -3;
	obj_lib["cube2"] = cube2;
	pso_objs["pso2"].push_back("cube2");
	auto sphere = std::make_shared<ObjectDx12>(cb_slot++, mesh_lib["sphere"]);
	sphere->transform.pos = Rehenz::Vector(0, 2.4f, 0);
	sphere->transform.scale = Rehenz::Vector(0.6f, 0.6f, 0.6f);
	obj_lib["sphere"] = sphere;
	pso_objs["pso2"].push_back("sphere");

	// init camera
	camera_trans.pos = Rehenz::Vector(1.2f, 1.6f, -4, 0);
	camera_trans.axes = Rehenz::AircraftAxes(0.4f, -0.3f, 0);
	camera_proj.z_far = 100;
	camera_proj.aspect = device->vp.Width / device->vp.Height;

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
	device->cmd_list->SetGraphicsRootDescriptorTable(1, frc.GetFrameCbvGpu());
	if (!frc.cb_light->CopyData(0, cb_light))
		return false;
	device->cmd_list->SetGraphicsRootDescriptorTable(2, frc.GetLightCbvGpu());

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
