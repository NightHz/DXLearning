#include "dx9.h"

IDirect3DDevice9* CreateSimpleDx9Device(SimpleWindow* window)
{
	// get IDirect3D9 interface
	IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	// check device
	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	DWORD vp;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// fill D3DPRESENT_PARAMETERS struct
	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.BackBufferWidth = window->GetWidth();
	d3dpp.BackBufferHeight = window->GetHeight();
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = window->GetHwnd();
	d3dpp.Windowed = true;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// create Direct3DDevice9
	HRESULT hr;
	IDirect3DDevice9* device = nullptr;
	hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window->GetHwnd(),
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device);

	// release IDirect3D9
	if (d3d9)
	{
		d3d9->Release();
		d3d9 = nullptr;
	}

	if (FAILED(hr))
		return nullptr;
	else
		return device;
}
