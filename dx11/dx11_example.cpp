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
library<VertexShader> vses;
library<PixelShader> pses;
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
	meshes["cube_xyz"] = Mesh::CreateTriangleXYZ(infra->device.Get());
	for (auto& p : meshes)
	{
		if (p.second == nullptr)
			return 1;
	}

	// vses
	vses["vs0"] = VertexShader::CompileVS(infra->device.Get(), L"vs0.hlsl");
	for (auto& p : vses)
	{
		if (p.second == nullptr)
			return 1;
	}

	// pses
	pses["ps0"] = PixelShader::CompilePS(infra->device.Get(), L"ps0.hlsl");
	for (auto& p : pses)
	{
		if (p.second == nullptr)
			return 1;
	}

	// objs
	auto cube = std::make_shared<Object>(infra->device.Get(), meshes["cube_xyz"], vses["vs0"], pses["ps0"]);
	objs["cube"] = cube;
	for (auto& p : objs)
	{
		if (!*p.second)
			return 1;
	}

	// cams
	auto cam = std::make_shared<Camera>(infra->device.Get(), infra->sc.Get(),
		static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));
	cams["cam"] = cam;
	for (auto& p : cams)
	{
		if (!*p.second)
			return 1;
	}

	// control_value
	control_value["bg_r"] = 0.7804f;
	control_value["bg_g"] = 0.8627f;
	control_value["bg_b"] = 0.4078f;

	return 0;
}

int dx11_control(Infrastructure* infra)
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

	first = false;
	return 0;
}


int dx11_render(Infrastructure* infra)
{
	HRESULT hr = 0;

	// clear and set camera
	auto& cam = cams["cam"];
	cam->Clear(infra->context.Get(), control_value["bg_r"], control_value["bg_g"], control_value["bg_b"], 0);
	cam->SetContext(infra->context.Get());

	// draw
	for (auto& p : objs)
	{
		auto& obj = p.second;
		obj->Draw(infra->context.Get());
	}

	// present
	hr = infra->sc->Present(0, 0);
	if (FAILED(hr))
		return 1;

	return 0;
}

int dx11_example()
{
	wcout.imbue(std::locale("", LC_CTYPE)); // set to system code

	auto window = std::make_shared<Rehenz::SimpleWindowWithFC>(GetModuleHandle(nullptr), 800, 600, "dx11");
	if (!window->CheckWindowState())
		return 1;
	cout << "finish create window" << endl;

	auto infra = CreateSimpleD3d11Device(window.get());
	if (infra == nullptr)
		return 1;
	cout << "finish create dx11 device" << endl;

	if (dx11_setup(window.get(), infra.get()) != 0)
		return 1;
	cout << "finish setup" << endl;

	while (true)
	{
		// control
		if (dx11_control(infra.get()) != 0)
			return 1;

		// render
		if (dx11_render(infra.get()) != 0)
			return 1;

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
