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
std::unordered_map<std::string, int> control_value;

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

	return 0;
}

int dx11_control(Infrastructure* infra)
{
	HRESULT hr = 0;
	static bool first = true;

	first = false;
	return 0;
}


int dx11_render(Infrastructure* infra)
{
	HRESULT hr = 0;

	// clear
	float bg_color[] = { 0.7804f, 0.8627f, 0.4078f, 0 };
	infra->context->ClearRenderTargetView(infra->rtv.Get(), bg_color);

	// draw
	for (auto& p : objs)
	{
		auto& obj = p.second;
		if (!obj->Draw(infra->context.Get()))
			return 1;
	}

	// present
	infra->sc->Present(0, 0);

	return 0;
}

int dx11_example()
{
	HRESULT hr = 0;
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
