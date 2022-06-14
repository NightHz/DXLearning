#include <iostream>
#include "dx12.h"
#include "Rehenz/input.h"

using std::cout;
using std::wcout;
using std::endl;
using namespace Dx12;
using Rehenz::KeyIsDown;

Rehenz::Mouse mouse;

template <typename T>
using base_library = std::unordered_map<std::string, T>;
base_library<std::shared_ptr<PipelineStateDx12>> pso_lib;
base_library<std::shared_ptr<std::vector<D3D12_INPUT_ELEMENT_DESC>>> il_lib;
base_library<ComPtr<ID3DBlob>> shader_lib;
base_library<std::shared_ptr<MeshDx12>> mesh_lib;
base_library<std::shared_ptr<ObjectDx12>> obj_lib;


void update(DeviceDx12* device, float dt)
{
	// control camera
	float cam_move_dis = 5 * dt;
	float cam_rotate_angle = 0.3f * dt;
	if (KeyIsDown('W')) device->camera_trans.pos += device->camera_trans.GetFrontInGround() * cam_move_dis;
	else if (KeyIsDown('S')) device->camera_trans.pos -= device->camera_trans.GetFrontInGround() * cam_move_dis;
	if (KeyIsDown('A')) device->camera_trans.pos -= device->camera_trans.GetRightInGround() * cam_move_dis;
	else if (KeyIsDown('D')) device->camera_trans.pos += device->camera_trans.GetRightInGround() * cam_move_dis;
	if (KeyIsDown(VK_SPACE)) device->camera_trans.pos.y += cam_move_dis;
	else if (KeyIsDown(VK_LSHIFT)) device->camera_trans.pos.y -= cam_move_dis;
	if (KeyIsDown(VK_MBUTTON))
	{
		device->camera_trans.axes.pitch += cam_rotate_angle * mouse.GetMoveY();
		device->camera_trans.axes.yaw += cam_rotate_angle * mouse.GetMoveX();
		mouse.SetToPrev();
	}
}

bool init(DeviceDx12* device)
{
	// init input layout
	std::vector<D3D12_INPUT_ELEMENT_DESC> il;
	il.resize(2);
	il[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	il[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	il_lib["pos+color"] = std::make_shared<std::vector<D3D12_INPUT_ELEMENT_DESC>>(std::move(il));

	// init shader
	shader_lib["vs_transform"] = UtilDx12::CompileShaderFile(L"dx12_vs_transform.hlsl", "vs");
	shader_lib["ps_color"] = UtilDx12::CompileShaderFile(L"dx12_ps_color.hlsl", "ps");
	for (auto& p : shader_lib)
	{
		if (!p.second)
			return false;
	}

	// init pso
	std::shared_ptr<PipelineStateDx12> pso;
	pso = std::make_shared<PipelineStateDx12>();
	pso->SetRootSignature(device->root_sig.Get());
	pso->SetInputLayout(*il_lib["pos+color"]);
	pso->SetVS(shader_lib["vs_transform"].Get());
	pso->SetPS(shader_lib["ps_color"].Get());
	if (!pso->CreatePSO(device))
		return false;
	pso_lib["pso1"] = pso;

	// init meshs
	mesh_lib["cube"] = MeshDx12::CreateCube(device);
	for (auto& p : mesh_lib)
	{
		if (!p.second)
			return false;
	}

	// init objs
	obj_lib["cube"] = std::make_shared<ObjectDx12>(mesh_lib["cube"]);

	// init camera
	device->camera_trans.pos = Rehenz::Vector(1.2f, 1.6f, -4, 0);
	device->camera_trans.axes = Rehenz::AircraftAxes(0.4f, -0.3f, 0);

	return true;
}

bool clean_after_init()
{
	mesh_lib["cube"]->FreeUploader();

	return true;
}

bool draw(DeviceDx12* device)
{
	device->cmd_list->SetPipelineState(pso_lib["pso1"]->pso.Get());

	if (!obj_lib["cube"]->Draw(device))
		return false;

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
	clean();
	device = nullptr;
	window = nullptr;

	return 0;
}
