#include "dx9.h"

namespace Dx9
{
	IDirect3DDevice9* CreateSimpleDx9Device(SimpleWindow* window)
	{
		HRESULT hr;

		// get IDirect3D9 interface
		IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

		// check device
		/*
		D3DCAPS9 caps;
		hr = d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
		if (FAILED(hr))
		{
			d3d9->Release();
			return nullptr;
		}
		DWORD vp;
		if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
			vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		*/

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
		IDirect3DDevice9* device = nullptr;
		hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window->GetHwnd(),
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device);
		if (FAILED(hr))
		{
			d3d9->Release();
			return nullptr;
		}

		// release IDirect3D9
		d3d9->Release();
		return device;
	}

	Mesh::Mesh()
	{
		mesh = nullptr;
		vb = nullptr;
		ib = nullptr;
		vertex_count = 0;
		vertex_size = 0;
		fvf = 0;
		index_count = 0;
	}

	Mesh::~Mesh()
	{
		if (mesh)
			mesh->Release();
		if (vb)
			vb->Release();
		if (ib)
			ib->Release();
	}

	bool Mesh::Draw(IDirect3DDevice9* device)
	{
		HRESULT hr;

		if (mesh)
		{
			//draw
			hr = mesh->DrawSubset(0);
			if (FAILED(hr))
				return false;
		}
		if (vb)
		{
			// set vb
			hr = device->SetStreamSource(0, vb, 0, vertex_size);
			if (FAILED(hr))
				return false;
			hr = device->SetFVF(fvf);
			if (FAILED(hr))
				return false;

			// set ib
			if (ib)
			{
				hr = device->SetIndices(ib);
				if (FAILED(hr))
					return false;
			}

			// draw
			if (ib)
				hr = device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertex_count, 0, index_count / 3);
			else
				hr = device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, vertex_count / 3);
			if (FAILED(hr))
				return false;
		}

		return true;
	}

	std::shared_ptr<Mesh> Mesh::CreateCubeXYZ(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		// create buffer
		mesh->vertex_size = sizeof(VertexXYZ);
		mesh->fvf = VertexXYZ::FVF;
		mesh->vertex_count = 8;
		mesh->index_count = 36;
		hr = device->CreateVertexBuffer(8 * sizeof(VertexXYZ),
			D3DUSAGE_WRITEONLY, VertexXYZ::FVF, D3DPOOL_MANAGED, &mesh->vb, 0);
		if (FAILED(hr))
			return nullptr;
		hr = device->CreateIndexBuffer(12 * 3 * sizeof(WORD),
			D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mesh->ib, 0);
		if (FAILED(hr))
			return nullptr;

		// fill buffer
		VertexXYZ* vertices;
		WORD* indexes;
		hr = mesh->vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0);
		if (FAILED(hr))
			return nullptr;
		// xyz
		vertices[0] = VertexXYZ(-1, -1, 1);
		vertices[1] = VertexXYZ(1, -1, 1);
		vertices[2] = VertexXYZ(-1, 1, 1);
		vertices[3] = VertexXYZ(1, 1, 1);
		vertices[4] = VertexXYZ(-1, -1, -1);
		vertices[5] = VertexXYZ(1, -1, -1);
		vertices[6] = VertexXYZ(-1, 1, -1);
		vertices[7] = VertexXYZ(1, 1, -1);
		hr = mesh->vb->Unlock();
		if (FAILED(hr))
			return nullptr;
		hr = mesh->ib->Lock(0, 0, reinterpret_cast<void**>(&indexes), 0);
		if (FAILED(hr))
			return nullptr;
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
		hr = mesh->ib->Unlock();
		if (FAILED(hr))
			return nullptr;

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateCubeColor(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		// create buffer
		mesh->vertex_size = sizeof(VertexColor);
		mesh->fvf = VertexColor::FVF;
		mesh->vertex_count = 8;
		mesh->index_count = 36;
		hr = device->CreateVertexBuffer(8 * sizeof(VertexColor),
			D3DUSAGE_WRITEONLY, VertexColor::FVF, D3DPOOL_MANAGED, &mesh->vb, 0);
		if (FAILED(hr))
			return nullptr;
		hr = device->CreateIndexBuffer(12 * 3 * sizeof(WORD),
			D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mesh->ib, 0);
		if (FAILED(hr))
			return nullptr;

		// fill buffer
		VertexColor* vertices;
		WORD* indexes;
		hr = mesh->vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0);
		if (FAILED(hr))
			return nullptr;
		// xyz
		vertices[0] = VertexColor(-1, -1, 1);
		vertices[1] = VertexColor(1, -1, 1);
		vertices[2] = VertexColor(-1, 1, 1);
		vertices[3] = VertexColor(1, 1, 1);
		vertices[4] = VertexColor(-1, -1, -1);
		vertices[5] = VertexColor(1, -1, -1);
		vertices[6] = VertexColor(-1, 1, -1);
		vertices[7] = VertexColor(1, 1, -1);
		// color
		vertices[0].color = D3DCOLOR_XRGB(0, 0, 0);
		vertices[1].color = D3DCOLOR_XRGB(0xff, 0, 0);
		vertices[2].color = D3DCOLOR_XRGB(0, 0xff, 0);
		vertices[3].color = D3DCOLOR_XRGB(0xff, 0xff, 0);
		vertices[4].color = D3DCOLOR_XRGB(0, 0, 0xff);
		vertices[5].color = D3DCOLOR_XRGB(0xff, 0, 0xff);
		vertices[6].color = D3DCOLOR_XRGB(0, 0xff, 0xff);
		vertices[7].color = D3DCOLOR_XRGB(0xff, 0xff, 0xff);
		hr = mesh->vb->Unlock();
		if (FAILED(hr))
			return nullptr;
		hr = mesh->ib->Lock(0, 0, reinterpret_cast<void**>(&indexes), 0);
		if (FAILED(hr))
			return nullptr;
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
		hr = mesh->ib->Unlock();
		if (FAILED(hr))
			return nullptr;

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateCubeNormal(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		device;
		hr;

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateD3DXTeapot(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		hr = D3DXCreateTeapot(device, &mesh->mesh, nullptr);
		//D3DXCreateBox(device, 2, 2, 2, &mesh_cube, nullptr);
		if (FAILED(hr))
			return nullptr;

		return mesh;
	}

	Object::Object(std::shared_ptr<Mesh> _mesh)
	{
		mesh = _mesh;
		mat.Ambient = D3DXCOLOR(1, 1, 1, 1);
		mat.Diffuse = D3DXCOLOR(1, 1, 1, 1);
		mat.Specular = D3DXCOLOR(1, 1, 1, 1);
		mat.Emissive = D3DXCOLOR(0, 0, 0, 1);
		mat.Power = 8;
		x = y = z = 0;
		phi = theta = psi = 0;
		sx = sy = sz = 1;
	}

	Object::~Object()
	{
	}

	bool Object::Transform(IDirect3DDevice9* device)
	{
		HRESULT hr;

		// compute matrix
		D3DXMATRIX mat_world;
		D3DXMATRIX mat_scale, mat_rotation, mat_position;
		// scale
		D3DXMatrixScaling(&mat_scale, sx, sy, sz);
		// rotation
		D3DXMATRIX mat_phi, mat_theta, mat_psi;
		D3DXMatrixRotationY(&mat_phi, phi);
		D3DXMatrixRotationZ(&mat_theta, theta);
		D3DXMatrixRotationY(&mat_psi, psi);
		mat_rotation = mat_phi * mat_theta * mat_psi;
		// translation
		D3DXMatrixTranslation(&mat_position, x, y, z);
		mat_world = mat_scale * mat_rotation * mat_position;

		// set transform
		hr = device->SetTransform(D3DTS_WORLD, &mat_world);
		if (FAILED(hr))
			return false;

		return true;
	}

	bool Object::Draw(IDirect3DDevice9* device)
	{
		HRESULT hr;

		// tranform
		if (!Transform(device))
			return false;

		// set material
		hr = device->SetMaterial(&mat);
		if (FAILED(hr))
			return false;

		// draw
		if (!mesh->Draw(device))
			return false;

		return true;
	}

	Camera::Camera()
		: pos(0, 0, 5), at(0, 0, 0), up(0, 1, 0)
	{
		fovy = D3DX_PI * 0.5f;
		aspect = 1;
		znear = 1;
		zfar = 500;
	}

	Camera::~Camera()
	{
	}

	bool Camera::Transform(IDirect3DDevice9* device)
	{
		HRESULT hr;

		// set camera
		D3DXMATRIX mat_view;
		D3DXMatrixLookAtLH(&mat_view, &pos, &at, &up);
		hr = device->SetTransform(D3DTS_VIEW, &mat_view);
		if (FAILED(hr))
			return false;

		// set projection
		D3DXMATRIX mat_project;
		D3DXMatrixPerspectiveFovLH(&mat_project, fovy, aspect, znear, zfar);
		hr = device->SetTransform(D3DTS_PROJECTION, &mat_project);
		if (FAILED(hr))
			return false;

		return true;
	}

}
