#include <iostream>
#include "window.h"
#include "input.h"
#include "dx9.h"
#include "fps_counter.h"
#include <timeapi.h>
#include <string>

using std::cout;
using std::endl;
using std::cin;

int dx9_example()
{
	HRESULT hr;

	SimpleWindow window(GetModuleHandle(nullptr), 800, 600, "dx9");
	if (!window.CheckWindowState())
		return 1;
	cout << "finish create window" << endl;

	IDirect3DDevice9* device = CreateSimpleDx9Device(&window);
	if (!device)
		return 1;
	cout << "finish create dx9 device" << endl;

	Rehenz::FpsCounter fps_counter(timeGetTime);
	auto updateFps = [&window](Rehenz::uint fps) { window.SetTitle((std::string("dx9 fps:") + std::to_string(fps)).c_str()); };
	fps_counter.UpdateFpsCallback = updateFps;
	cout << "start fps_counter" << endl;

	// setup
	// create buffer
	IDirect3DVertexBuffer9* vb;
	IDirect3DIndexBuffer9* ib;
	hr = device->CreateVertexBuffer(8 * sizeof(Vertex),
		D3DUSAGE_WRITEONLY, Vertex::FVF, D3DPOOL_MANAGED, &vb, 0);
	if (FAILED(hr))
		return 1;
	hr = device->CreateIndexBuffer(12 * 3 * sizeof(WORD),
		D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib, 0);
	if (FAILED(hr))
		return 1;

	// fill buffer
	Vertex* vertices;
	WORD* indexes;
	hr = vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0);
	if (FAILED(hr))
		return 1;
	vertices[0] = Vertex(-1, -1, 1);
	vertices[1] = Vertex(1, -1, 1);
	vertices[2] = Vertex(-1, 1, 1);
	vertices[3] = Vertex(1, 1, 1);
	vertices[4] = Vertex(-1, -1, -1);
	vertices[5] = Vertex(1, -1, -1);
	vertices[6] = Vertex(-1, 1, -1);
	vertices[7] = Vertex(1, 1, -1);
	hr = vb->Unlock();
	if (FAILED(hr))
		return 1;
	hr = ib->Lock(0, 0, reinterpret_cast<void**>(&indexes), 0);
	if (FAILED(hr))
		return 1;
	indexes[0] = 0; indexes[1] = 1; indexes[2] = 3;
	indexes[3] = 0; indexes[4] = 3; indexes[5] = 2;
	indexes[6] = 4; indexes[7] = 6; indexes[8] = 7;
	indexes[9] = 4; indexes[10] = 7; indexes[11] = 5;
	indexes[12] = 0; indexes[13] = 4; indexes[14] = 5;
	indexes[15] = 0; indexes[16] = 5; indexes[17] = 1;
	indexes[18] = 2; indexes[19] = 3; indexes[20] = 7;
	indexes[21] = 2; indexes[22] = 7; indexes[23] = 6;
	indexes[24] = 1; indexes[25] = 5; indexes[26] = 7;
	indexes[27] = 1; indexes[28] = 7; indexes[29] = 3;
	indexes[30] = 0; indexes[31] = 2; indexes[32] = 6;
	indexes[33] = 0; indexes[34] = 6; indexes[35] = 4;
	hr = ib->Unlock();
	if (FAILED(hr))
		return 1;

	ID3DXMesh* mesh;
	D3DXCreateTeapot(device, &mesh, nullptr);
	//D3DXCreateBox(device, 2, 2, 2, &mesh, nullptr);

	cout << "finish setup" << endl;

	// control
	float cx, cy;
	cx = cy = 0;
	float x, y, z;
	x = y = z = 0;
	float rx, ry;
	rx = ry = D3DX_PI * 0.25f;

	while (true)
	{
		// clear
		D3DCOLOR color = 0x00ffffff; // white
		if (KeyIsDown('1')) color -= 0x00ff0000; // - red
		if (KeyIsDown('2')) color -= 0x0000ff00; // - green
		if (KeyIsDown('3')) color -= 0x000000ff; // - blue
		device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);

		// fixed pipeline
		// set rendering state
		if (KeyIsDown('4'))
			hr = device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		else if (KeyIsDown('5'))
			hr = device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		if (FAILED(hr))
			return 1;

		// set camera
		if (KeyIsDown('W')) cy += 0.1f;
		else if (KeyIsDown('S')) cy -= 0.1f;
		if (KeyIsDown('A')) cx += 0.1f;
		else if (KeyIsDown('D')) cx -= 0.1f;
		D3DXVECTOR3 cam_pos(cx, cy, 5);
		D3DXVECTOR3 cam_at(0, 0, 0);
		D3DXVECTOR3 cam_up(0, 1, 0);
		D3DXMATRIX mat_view;
		D3DXMatrixLookAtLH(&mat_view, &cam_pos, &cam_at, &cam_up);
		hr = device->SetTransform(D3DTS_VIEW, &mat_view);
		if (FAILED(hr))
			return 1;

		// set projection
		D3DXMATRIX mat_project;
		D3DXMatrixPerspectiveFovLH(&mat_project, D3DX_PI * 0.5f, static_cast<float>(window.GetWidth()) / window.GetHeight(), 1, 500);
		hr = device->SetTransform(D3DTS_PROJECTION, &mat_project);
		if (FAILED(hr))
			return 1;

		// set transform
		D3DXMATRIX mat_world;
		D3DXMATRIX mat_scale, mat_rotation, mat_position;
		D3DXMatrixScaling(&mat_scale, 1, 1, 1);
		if (KeyIsDown('I')) rx -= 0.05f;
		else if (KeyIsDown('K')) rx += 0.05f;
		if (KeyIsDown('J')) ry += 0.05f;
		else if (KeyIsDown('L')) ry -= 0.05f;
		D3DXMATRIX mat_rx, mat_ry;
		D3DXMatrixRotationX(&mat_rx, rx);
		D3DXMatrixRotationY(&mat_ry, ry);
		mat_rotation = mat_rx * mat_ry;
		if (KeyIsDown('T')) y += 0.1f;
		else if (KeyIsDown('G')) y -= 0.1f;
		if (KeyIsDown('F')) x += 0.1f;
		else if (KeyIsDown('H')) x -= 0.1f;
		if (KeyIsDown('R')) z += 0.1f;
		else if (KeyIsDown('Y')) z -= 0.1f;
		D3DXMatrixTranslation(&mat_position, x, y, z);
		mat_world = mat_scale * mat_rotation * mat_position;
		hr = device->SetTransform(D3DTS_WORLD, &mat_world);
		if (FAILED(hr))
			return 1;

		// begin
		hr = device->BeginScene();
		if (FAILED(hr))
			return 1;

		if (!KeyIsDown(VK_RETURN))
		{
			// set buffer and draw
			hr = device->SetStreamSource(0, vb, 0, sizeof(Vertex));
			if (FAILED(hr))
				return 1;
			hr = device->SetFVF(Vertex::FVF);
			if (FAILED(hr))
				return 1;
			hr = device->SetIndices(ib);
			if (FAILED(hr))
				return 1;
			hr = device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
			//hr = device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
			if (FAILED(hr))
				return 1;
		}
		else
		{
			mesh->DrawSubset(0);
		}

		// end
		hr = device->EndScene();
		if (FAILED(hr))
			return 1;

		// present
		device->Present(nullptr, nullptr, nullptr, nullptr);
		fps_counter.Present();

		// msg
		SimpleMessageProcess();
		if (!window.CheckWindowState() || KeyIsDown('Q'))
			break;
	}

	// release
	mesh->Release();
	vb->Release();
	ib->Release();
	device->Release();
	cout << "finish release" << endl;

	return 0;
}

int main()
{
	cout << "DirectX learning ..." << endl;
	return dx9_example();
}