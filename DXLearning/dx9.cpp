#include "dx9.h"
#include <fstream>
#include <sstream>

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
		subset_count = 0;
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
			for (unsigned int i = 0; i < subset_count; i++)
			{
				hr = mesh->DrawSubset(i);
				if (FAILED(hr))
					return false;
			}
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

	std::shared_ptr<Mesh> Mesh::CreateTetrahedronNormal(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		// create buffer
		mesh->vertex_size = sizeof(VertexNormal);
		mesh->fvf = VertexNormal::FVF;
		mesh->vertex_count = 12;
		hr = device->CreateVertexBuffer(4 * 3 * sizeof(VertexNormal),
			D3DUSAGE_WRITEONLY, VertexNormal::FVF, D3DPOOL_MANAGED, &mesh->vb, 0);
		if (FAILED(hr))
			return nullptr;

		// fill buffer
		VertexNormal* vertices;
		hr = mesh->vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0);
		if (FAILED(hr))
			return nullptr;
		// xyz
		vertices[0] = VertexNormal(-1, -1, 1);
		vertices[1] = VertexNormal(1, -1, 1);
		vertices[2] = VertexNormal(0, 1, 0);
		vertices[3] = vertices[1];
		vertices[4] = VertexNormal(0, -1, -1);
		vertices[5] = vertices[2];
		vertices[6] = vertices[4];
		vertices[7] = vertices[0];
		vertices[8] = vertices[2];
		vertices[9] = vertices[0];
		vertices[10] = vertices[4];
		vertices[11] = vertices[1];
		// normal
		for (int i = 0; i < 12; i += 3)
		{
			D3DXVECTOR3 normal = TriangleNormal(vertices[i], vertices[i + 1], vertices[i + 2]);
			VertexSetNormal(vertices[i], normal);
			VertexSetNormal(vertices[i + 1], normal);
			VertexSetNormal(vertices[i + 2], normal);
		}
		hr = mesh->vb->Unlock();
		if (FAILED(hr))
			return nullptr;

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateTetrahedronNormalColor(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		// create buffer
		mesh->vertex_size = sizeof(VertexNormalColor);
		mesh->fvf = VertexNormalColor::FVF;
		mesh->vertex_count = 12;
		hr = device->CreateVertexBuffer(4 * 3 * sizeof(VertexNormalColor),
			D3DUSAGE_WRITEONLY, VertexNormalColor::FVF, D3DPOOL_MANAGED, &mesh->vb, 0);
		if (FAILED(hr))
			return nullptr;

		// fill buffer
		VertexNormalColor* vertices;
		hr = mesh->vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0);
		if (FAILED(hr))
			return nullptr;
		// xyz
		vertices[0] = VertexNormalColor(-1, -1, 1);
		vertices[1] = VertexNormalColor(1, -1, 1);
		vertices[2] = VertexNormalColor(0, 1, 0);
		vertices[4] = VertexNormalColor(0, -1, -1);
		// color
		vertices[0].color = D3DCOLOR_XRGB(0xff, 0, 0);
		vertices[1].color = D3DCOLOR_XRGB(0, 0xff, 0);
		vertices[2].color = D3DCOLOR_XRGB(0x80, 0x80, 0x80);
		vertices[4].color = D3DCOLOR_XRGB(0, 0, 0xff);
		vertices[3] = vertices[1];
		vertices[5] = vertices[2];
		vertices[6] = vertices[4];
		vertices[7] = vertices[0];
		vertices[8] = vertices[2];
		vertices[9] = vertices[0];
		vertices[10] = vertices[4];
		vertices[11] = vertices[1];
		// normal
		for (int i = 0; i < 12; i += 3)
		{
			D3DXVECTOR3 normal = TriangleNormal(vertices[i], vertices[i + 1], vertices[i + 2]);
			VertexSetNormal(vertices[i], normal);
			VertexSetNormal(vertices[i + 1], normal);
			VertexSetNormal(vertices[i + 2], normal);
		}
		hr = mesh->vb->Unlock();
		if (FAILED(hr))
			return nullptr;

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateCubeNormalColorTex1(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		// create buffer
		mesh->vertex_size = sizeof(VertexNormalColorTex1);
		mesh->fvf = VertexNormalColorTex1::FVF;
		mesh->vertex_count = 36;
		hr = device->CreateVertexBuffer(12 * 3 * sizeof(VertexNormalColorTex1),
			D3DUSAGE_WRITEONLY, VertexNormalColorTex1::FVF, D3DPOOL_MANAGED, &mesh->vb, 0);
		if (FAILED(hr))
			return nullptr;

		// base cube
		VertexNormalColorTex1 v[8];
		// xyz
		v[0] = VertexNormalColorTex1(-1, -1, 1);
		v[1] = VertexNormalColorTex1(1, -1, 1);
		v[2] = VertexNormalColorTex1(-1, 1, 1);
		v[3] = VertexNormalColorTex1(1, 1, 1);
		v[4] = VertexNormalColorTex1(-1, -1, -1);
		v[5] = VertexNormalColorTex1(1, -1, -1);
		v[6] = VertexNormalColorTex1(-1, 1, -1);
		v[7] = VertexNormalColorTex1(1, 1, -1);
		// color
		v[0].color = D3DCOLOR_XRGB(0, 0, 0);
		v[1].color = D3DCOLOR_XRGB(0xff, 0, 0);
		v[2].color = D3DCOLOR_XRGB(0, 0xff, 0);
		v[3].color = D3DCOLOR_XRGB(0xff, 0xff, 0);
		v[4].color = D3DCOLOR_XRGB(0, 0, 0xff);
		v[5].color = D3DCOLOR_XRGB(0xff, 0, 0xff);
		v[6].color = D3DCOLOR_XRGB(0, 0xff, 0xff);
		v[7].color = D3DCOLOR_XRGB(0xff, 0xff, 0xff);

		// fill buffer
		VertexNormalColorTex1* vertices;
		hr = mesh->vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0);
		if (FAILED(hr))
			return nullptr;
		// xyz and color
		vertices[0] = v[0]; vertices[1] = v[1]; vertices[2] = v[3];
		vertices[3] = v[0]; vertices[4] = v[3]; vertices[5] = v[2];
		vertices[6] = v[4]; vertices[7] = v[6]; vertices[8] = v[7];
		vertices[9] = v[4]; vertices[10] = v[7]; vertices[11] = v[5];
		vertices[12] = v[0]; vertices[13] = v[4]; vertices[14] = v[5];
		vertices[15] = v[0]; vertices[16] = v[5]; vertices[17] = v[1];
		vertices[18] = v[2]; vertices[19] = v[3]; vertices[20] = v[7];
		vertices[21] = v[2]; vertices[22] = v[7]; vertices[23] = v[6];
		vertices[24] = v[1]; vertices[25] = v[5]; vertices[26] = v[7];
		vertices[27] = v[1]; vertices[28] = v[7]; vertices[29] = v[3];
		vertices[30] = v[0]; vertices[31] = v[2]; vertices[32] = v[6];
		vertices[33] = v[0]; vertices[34] = v[6]; vertices[35] = v[4];
		// normal
		for (int i = 0; i < 36; i += 3)
		{
			D3DXVECTOR3 normal = TriangleNormal(vertices[i], vertices[i + 1], vertices[i + 2]);
			VertexSetNormal(vertices[i], normal);
			VertexSetNormal(vertices[i + 1], normal);
			VertexSetNormal(vertices[i + 2], normal);
		}
		// tex1
		const float H = 3, L = 0;
		VertexSetTex1(vertices[0], H, H); VertexSetTex1(vertices[1], L, H); VertexSetTex1(vertices[2], L, L);
		VertexSetTex1(vertices[3], H, H); VertexSetTex1(vertices[4], L, L); VertexSetTex1(vertices[5], H, L);
		VertexSetTex1(vertices[6], L, H); VertexSetTex1(vertices[7], L, L); VertexSetTex1(vertices[8], H, L);
		VertexSetTex1(vertices[9], L, H); VertexSetTex1(vertices[10], H, L); VertexSetTex1(vertices[11], H, H);
		VertexSetTex1(vertices[12], L, L); VertexSetTex1(vertices[13], L, H); VertexSetTex1(vertices[14], H, H);
		VertexSetTex1(vertices[15], L, L); VertexSetTex1(vertices[16], H, H); VertexSetTex1(vertices[17], H, L);
		VertexSetTex1(vertices[18], L, L); VertexSetTex1(vertices[19], H, L); VertexSetTex1(vertices[20], H, H);
		VertexSetTex1(vertices[21], L, L); VertexSetTex1(vertices[22], H, H); VertexSetTex1(vertices[23], L, H);
		VertexSetTex1(vertices[24], H, H); VertexSetTex1(vertices[25], L, H); VertexSetTex1(vertices[26], L, L);
		VertexSetTex1(vertices[27], H, H); VertexSetTex1(vertices[28], L, L); VertexSetTex1(vertices[29], H, L);
		VertexSetTex1(vertices[30], L, H); VertexSetTex1(vertices[31], L, L); VertexSetTex1(vertices[32], H, L);
		VertexSetTex1(vertices[33], L, H); VertexSetTex1(vertices[34], H, L); VertexSetTex1(vertices[35], H, H);
		hr = mesh->vb->Unlock();
		if (FAILED(hr))
			return nullptr;

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreatePlaneNormal(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		// create buffer
		mesh->vertex_size = sizeof(VertexNormal);
		mesh->fvf = VertexNormal::FVF;
		mesh->vertex_count = 6;
		hr = device->CreateVertexBuffer(2 * 3 * sizeof(VertexNormal),
			D3DUSAGE_WRITEONLY, VertexNormal::FVF, D3DPOOL_MANAGED, &mesh->vb, 0);
		if (FAILED(hr))
			return nullptr;

		// fill buffer
		VertexNormal* vertices;
		hr = mesh->vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0);
		if (FAILED(hr))
			return nullptr;
		// xyz
		vertices[0] = VertexNormal(-1, -1, 0);
		vertices[1] = VertexNormal(1, -1, 0);
		vertices[2] = VertexNormal(1, 1, 0);
		vertices[3] = VertexNormal(-1, -1, 0);
		vertices[4] = VertexNormal(1, 1, 0);
		vertices[5] = VertexNormal(-1, 1, 0);
		// normal
		D3DXVECTOR3 normal(0, 0, 1);
		VertexSetNormal(vertices[0], normal);
		VertexSetNormal(vertices[1], normal);
		VertexSetNormal(vertices[2], normal);
		VertexSetNormal(vertices[3], normal);
		VertexSetNormal(vertices[4], normal);
		VertexSetNormal(vertices[5], normal);
		hr = mesh->vb->Unlock();
		if (FAILED(hr))
			return nullptr;

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
		mesh->subset_count = 1;

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateD3DXText(IDirect3DDevice9* device, const std::string& text)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		HDC hdc = CreateCompatibleDC(0);

		LOGFONT lf;
		lf.lfHeight = 25;
		lf.lfWidth = 12;
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		lf.lfWeight = 500;
		lf.lfItalic = false;
		lf.lfUnderline = true;
		lf.lfStrikeOut = false;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = 0;
		lf.lfClipPrecision = 0;
		lf.lfQuality = 0;
		lf.lfPitchAndFamily = 0;
		strcpy_s(lf.lfFaceName, LF_FACESIZE, "Times New Roman");
		HFONT hfont = CreateFontIndirect(&lf);
		SelectObject(hdc, hfont);

		hr = D3DXCreateTextA(device, hdc, text.c_str(), 0.001f, 0.4f, &mesh->mesh, nullptr, nullptr);
		if (FAILED(hr))
		{
			DeleteDC(hdc);
			return nullptr;
		}
		mesh->subset_count = 1;

		DeleteDC(hdc);

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateFromFile(IDirect3DDevice9* device, const std::string& file_path)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		// open file
		std::ifstream file;
		file.open(file_path);
		if (!file.is_open())
			return nullptr;
		std::string line;

		// count
		int v_count = 0, f_count = 0;
		while (std::getline(file, line))
		{
			std::istringstream ss(line);
			std::string word;
			if (!(ss >> word)) continue;
			if (word == "v") v_count++;
			else if (word == "f") f_count++;
		}

		// create mesh
		v_count += 1; // .obj file first index is 1
		hr = D3DXCreateMeshFVF(f_count, v_count, D3DXMESH_MANAGED, VertexXYZ::FVF, device, &mesh->mesh);
		if (FAILED(hr))
		{
			file.close();
			return nullptr;
		}

		// lock buffer
		VertexXYZ* vertices;
		WORD* indexes;
		DWORD* attributes;
		hr = mesh->mesh->LockVertexBuffer(0, reinterpret_cast<void**>(&vertices));
		if (FAILED(hr))
		{
			file.close();
			return nullptr;
		}
		hr = mesh->mesh->LockIndexBuffer(0, reinterpret_cast<void**>(&indexes));
		if (FAILED(hr))
		{
			file.close();
			return nullptr;
		}
		hr = mesh->mesh->LockAttributeBuffer(0, &attributes);
		if (FAILED(hr))
		{
			file.close();
			return nullptr;
		}

		// write data
		file.clear();
		file.seekg(0);
		int v_i = 1, f_i = 0;
		bool f_continue = false;
		int subset_id = -1;
		while (std::getline(file, line))
		{
			std::istringstream ss(line);
			std::string word;
			if (!(ss >> word))
				continue;
			if (word == "v")
			{
				ss >> vertices[v_i].x >> vertices[v_i].y >> vertices[v_i].z;
				v_i++;
				f_continue = false;
			}
			else if (word == "f")
			{
				char c;
				WORD i;
				if (ss >> i) indexes[3 * f_i] = i; else ss.clear();
				ss >> c;
				if (ss >> i) i; else ss.clear();
				ss >> c;
				if (ss >> i) i; else ss.clear();
				if (ss >> i) indexes[3 * f_i + 1] = i; else ss.clear();
				ss >> c;
				if (ss >> i) i; else ss.clear();
				ss >> c;
				if (ss >> i) i; else ss.clear();
				if (ss >> i) indexes[3 * f_i + 2] = i; else ss.clear();
				ss >> c;
				if (ss >> i) i; else ss.clear();
				ss >> c;
				if (ss >> i) i; else ss.clear();
				if (!f_continue)
					subset_id++, f_continue = true;
				attributes[f_i] = subset_id;
				f_i++;
			}
		}
		mesh->subset_count = subset_id + 1;
		file.close();

		// unlcok buffer
		hr = mesh->mesh->UnlockAttributeBuffer();
		if (FAILED(hr))
			return nullptr;
		hr = mesh->mesh->UnlockIndexBuffer();
		if (FAILED(hr))
			return nullptr;
		hr = mesh->mesh->UnlockVertexBuffer();
		if (FAILED(hr))
			return nullptr;

		// optimize
		ID3DXBuffer* adjacency_info;
		hr = D3DXCreateBuffer(f_count * 3 * sizeof(DWORD), &adjacency_info);
		if (FAILED(hr))
			return nullptr;
		hr = mesh->mesh->GenerateAdjacency(0.001f, static_cast<DWORD*>(adjacency_info->GetBufferPointer()));
		if (FAILED(hr))
		{
			adjacency_info->Release();
			return nullptr;
		}
		hr = mesh->mesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_COMPACT | D3DXMESHOPT_VERTEXCACHE,
			static_cast<DWORD*>(adjacency_info->GetBufferPointer()), nullptr, nullptr, nullptr);
		if (FAILED(hr))
		{
			adjacency_info->Release();
			return nullptr;
		}
		adjacency_info->Release();

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateFromFileNormal(IDirect3DDevice9* device, const std::string& file_path)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		// open file
		std::ifstream file;
		file.open(file_path);
		if (!file.is_open())
			return nullptr;
		std::string line;

		// count
		int v_count = 0, vn_count = 0, f_count = 0;
		while (std::getline(file, line))
		{
			std::istringstream ss(line);
			std::string word;
			if (!(ss >> word)) continue;
			if (word == "v") v_count++;
			else if (word == "vn") vn_count++;
			else if (word == "f") f_count++;
		}

		// create mesh
		v_count += 1; // .obj file first index is 1
		vn_count += 1;
		hr = D3DXCreateMeshFVF(f_count, 3 * f_count, D3DXMESH_MANAGED, VertexNormal::FVF, device, &mesh->mesh);
		if (FAILED(hr))
		{
			file.close();
			return nullptr;
		}

		// lock buffer
		VertexNormal* vertices;
		WORD* indexes;
		DWORD* attributes;
		hr = mesh->mesh->LockVertexBuffer(0, reinterpret_cast<void**>(&vertices));
		if (FAILED(hr))
		{
			file.close();
			return nullptr;
		}
		hr = mesh->mesh->LockIndexBuffer(0, reinterpret_cast<void**>(&indexes));
		if (FAILED(hr))
		{
			file.close();
			return nullptr;
		}
		hr = mesh->mesh->LockAttributeBuffer(0, &attributes);
		if (FAILED(hr))
		{
			file.close();
			return nullptr;
		}

		// write data
		file.clear();
		file.seekg(0);
		float* v_info = new float[static_cast<size_t>(3) * v_count]{ 0 };
		float* vn_info = new float[static_cast<size_t>(3) * vn_count]{ 0 };
		int v_i = 1, vn_i = 1, f_i = 0;
		bool f_continue = false;
		int subset_id = -1;
		while (std::getline(file, line))
		{
			std::istringstream ss(line);
			std::string word;
			if (!(ss >> word))
				continue;
			if (word == "v")
			{
				ss >> v_info[3 * v_i] >> v_info[3 * v_i + 1] >> v_info[3 * v_i + 2];
				v_i++;
				f_continue = false;
			}
			else if (word == "vn")
			{
				ss >> vn_info[3 * vn_i] >> vn_info[3 * vn_i + 1] >> vn_info[3 * vn_i + 2];
				vn_i++;
			}
			else if (word == "f")
			{
				char c;
				WORD i;
				if (ss >> i)
				{
					indexes[3 * f_i] = static_cast<WORD>(3 * f_i);
					vertices[3 * f_i].x = v_info[3 * i];
					vertices[3 * f_i].y = v_info[3 * i + 1];
					vertices[3 * f_i].z = v_info[3 * i + 2];
				}
				else ss.clear();
				ss >> c;
				if (ss >> i) i; else ss.clear();
				ss >> c;
				if (ss >> i)
				{
					vertices[3 * f_i].nx = vn_info[3 * i];
					vertices[3 * f_i].ny = vn_info[3 * i + 1];
					vertices[3 * f_i].nz = vn_info[3 * i + 2];
				}
				else ss.clear();
				if (ss >> i)
				{
					indexes[3 * f_i + 1] = static_cast<WORD>(3 * f_i + 1);
					vertices[3 * f_i + 1].x = v_info[3 * i];
					vertices[3 * f_i + 1].y = v_info[3 * i + 1];
					vertices[3 * f_i + 1].z = v_info[3 * i + 2];
				}
				else ss.clear();
				ss >> c;
				if (ss >> i) i; else ss.clear();
				ss >> c;
				if (ss >> i)
				{
					vertices[3 * f_i + 1].nx = vn_info[3 * i];
					vertices[3 * f_i + 1].ny = vn_info[3 * i + 1];
					vertices[3 * f_i + 1].nz = vn_info[3 * i + 2];
				}
				else ss.clear();
				if (ss >> i)
				{
					indexes[3 * f_i + 2] = static_cast<WORD>(3 * f_i + 2);
					vertices[3 * f_i + 2].x = v_info[3 * i];
					vertices[3 * f_i + 2].y = v_info[3 * i + 1];
					vertices[3 * f_i + 2].z = v_info[3 * i + 2];
				}
				else ss.clear();
				ss >> c;
				if (ss >> i) i; else ss.clear();
				ss >> c;
				if (ss >> i)
				{
					vertices[3 * f_i + 2].nx = vn_info[3 * i];
					vertices[3 * f_i + 2].ny = vn_info[3 * i + 1];
					vertices[3 * f_i + 2].nz = vn_info[3 * i + 2];
				}
				else ss.clear();
				if (!f_continue)
					subset_id++, f_continue = true;
				attributes[f_i] = subset_id;
				f_i++;
			}
		}
		mesh->subset_count = subset_id + 1;
		file.close();
		delete[] v_info;
		delete[] vn_info;

		// unlcok buffer
		hr = mesh->mesh->UnlockAttributeBuffer();
		if (FAILED(hr))
			return nullptr;
		hr = mesh->mesh->UnlockIndexBuffer();
		if (FAILED(hr))
			return nullptr;
		hr = mesh->mesh->UnlockVertexBuffer();
		if (FAILED(hr))
			return nullptr;

		// optimize
		ID3DXBuffer* adjacency_info;
		hr = D3DXCreateBuffer(f_count * 3 * sizeof(DWORD), &adjacency_info);
		if (FAILED(hr))
			return nullptr;
		hr = mesh->mesh->GenerateAdjacency(0.001f, static_cast<DWORD*>(adjacency_info->GetBufferPointer()));
		if (FAILED(hr))
		{
			adjacency_info->Release();
			return nullptr;
		}
		hr = mesh->mesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_COMPACT | D3DXMESHOPT_VERTEXCACHE,
			static_cast<DWORD*>(adjacency_info->GetBufferPointer()), nullptr, nullptr, nullptr);
		if (FAILED(hr))
		{
			adjacency_info->Release();
			return nullptr;
		}
		adjacency_info->Release();

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

	D3DXMATRIX Object::ComputeTransform()
	{
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

		return mat_scale * mat_rotation * mat_position;
	}

	bool Object::Draw(IDirect3DDevice9* device)
	{
		HRESULT hr;

		// set transform
		D3DXMATRIX mat_world = ComputeTransform();
		hr = device->SetTransform(D3DTS_WORLD, &mat_world);
		if (FAILED(hr))
			return false;

		// set material
		hr = device->SetMaterial(&mat);
		if (FAILED(hr))
			return false;

		// set texture
		if (texture)
			hr = device->SetTexture(0, texture->tex);
		else
			hr = device->SetTexture(0, nullptr);
		if (FAILED(hr))
			return false;

		// draw
		if (mesh)
		{
			if (!mesh->Draw(device))
				return false;
		}

		return true;
	}

	bool Object::DrawMirror(IDirect3DDevice9* device, const D3DXPLANE& plane)
	{
		HRESULT hr;

		// set transform
		D3DXMATRIX mat_reflect;
		D3DXMatrixReflect(&mat_reflect, &plane);
		D3DXMATRIX mat_world = ComputeTransform() * mat_reflect;
		hr = device->SetTransform(D3DTS_WORLD, &mat_world);
		if (FAILED(hr))
			return false;

		// set material
		hr = device->SetMaterial(&mat);
		if (FAILED(hr))
			return false;

		// set texture
		if (texture)
			hr = device->SetTexture(0, texture->tex);
		else
			hr = device->SetTexture(0, nullptr);
		if (FAILED(hr))
			return false;

		// draw
		if (mesh)
		{
			if (!mesh->Draw(device))
				return false;
		}

		return true;
	}

	bool Object::DrawShadow(IDirect3DDevice9* device, const D3DXVECTOR4& light_dir, const D3DXPLANE& plane)
	{
		HRESULT hr;

		// set transform
		D3DXMATRIX mat_shadow;
		D3DXMatrixShadow(&mat_shadow, &light_dir, &plane); // the function have some errors
		mat_shadow *= -1;                                  // temporary fix
		D3DXMATRIX mat_world = ComputeTransform() * mat_shadow;
		hr = device->SetTransform(D3DTS_WORLD, &mat_world);
		if (FAILED(hr))
			return false;

		// set material
		D3DMATERIAL9 material;
		material.Ambient = D3DXCOLOR(0, 0, 0, 1);
		material.Diffuse = D3DXCOLOR(0, 0, 0, 0.5f);
		material.Specular = D3DXCOLOR(0, 0, 0, 1);
		material.Emissive = D3DXCOLOR(0, 0, 0, 1);
		material.Power = 8;
		hr = device->SetMaterial(&material);
		if (FAILED(hr))
			return false;

		// set texture
		hr = device->SetTexture(0, nullptr);
		if (FAILED(hr))
			return false;

		// draw
		if (mesh)
		{
			hr = device->SetRenderState(D3DRS_COLORVERTEX, false);
			if (FAILED(hr))
				return 1;
			if (!mesh->Draw(device))
				return false;
			hr = device->SetRenderState(D3DRS_COLORVERTEX, true);
			if (FAILED(hr))
				return 1;
		}

		return true;
	}

	Camera::Camera()
		: pos(0, 0, 5)
	{
		pitch = 0;
		yaw = D3DX_PI;
		fovy = D3DX_PI * 0.5f;
		aspect = 1;
		znear = 1;
		zfar = 500;
	}

	Camera::~Camera()
	{
	}

	void Camera::MoveFront(float d)
	{
		D3DXMATRIX mat_yaw;
		D3DXMatrixRotationY(&mat_yaw, -yaw);
		D3DXVECTOR4 z(0, 0, 1, 0), front;
		D3DXVec4Transform(&front, &z, &mat_yaw);
		pos.x += front.x * d;
		pos.z += front.z * d;
	}

	void Camera::MoveBack(float d)
	{
		MoveFront(-d);
	}

	void Camera::MoveLeft(float d)
	{
		MoveRight(-d);
	}

	void Camera::MoveRight(float d)
	{
		D3DXMATRIX mat_yaw;
		D3DXMatrixRotationY(&mat_yaw, -yaw);
		D3DXVECTOR4 x(1, 0, 0, 0), right;
		D3DXVec4Transform(&right, &x, &mat_yaw);
		pos.x += right.x * d;
		pos.z += right.z * d;
	}

	void Camera::MoveUp(float d)
	{
		pos.y += d;
	}

	void Camera::MoveDown(float d)
	{
		MoveUp(-d);
	}

	void Camera::YawLeft(float angle)
	{
		yaw += angle;
	}

	void Camera::YawRight(float angle)
	{
		YawLeft(-angle);
	}

	void Camera::PitchUp(float angle)
	{
		pitch += angle;
	}

	void Camera::PitchDown(float angle)
	{
		PitchUp(-angle);
	}

	bool Camera::Transform(IDirect3DDevice9* device)
	{
		HRESULT hr;

		// set camera
		D3DXMATRIX mat_position, mat_rotation;
		D3DXMatrixTranslation(&mat_position, -pos.x, -pos.y, -pos.z);
		D3DXMATRIX mat_pitch, mat_yaw;
		D3DXMatrixRotationY(&mat_yaw, yaw);
		D3DXMatrixRotationX(&mat_pitch, pitch);
		mat_rotation = mat_yaw * mat_pitch;
		D3DXMATRIX mat_view;
		mat_view = mat_position * mat_rotation;
		//D3DXMatrixLookAtLH(&mat_view, &pos, &at, &up);
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

	bool Camera::TransformReflect(IDirect3DDevice9* device, const D3DXPLANE& plane)
	{
		HRESULT hr;

		// set camera and mirror
		D3DXMATRIX mat_reflect;
		D3DXMatrixReflect(&mat_reflect, &plane);
		D3DXMATRIX mat_position, mat_rotation;
		D3DXMatrixTranslation(&mat_position, -pos.x, -pos.y, -pos.z);
		D3DXMATRIX mat_pitch, mat_yaw;
		D3DXMatrixRotationY(&mat_yaw, yaw);
		D3DXMatrixRotationX(&mat_pitch, pitch);
		mat_rotation = mat_yaw * mat_pitch;
		D3DXMATRIX mat_view;
		mat_view = mat_position * mat_rotation;
		//D3DXMatrixLookAtLH(&mat_view, &pos, &at, &up);
		D3DXMATRIX mat_view2 = mat_reflect * mat_view;
		hr = device->SetTransform(D3DTS_VIEW, &mat_view2);
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

	Texture::Texture()
	{
		tex = nullptr;
	}

	Texture::~Texture()
	{
		if (tex)
			tex->Release();
	}

	std::shared_ptr<Texture> Texture::CreateTexture(IDirect3DDevice9* device, const char* filename)
	{
		auto texture = std::shared_ptr<Texture>(new Texture());
		HRESULT hr;

		// create texture
		hr = D3DXCreateTextureFromFile(device, filename, &texture->tex);
		if (FAILED(hr))
			return nullptr;

		return texture;
	}

}
