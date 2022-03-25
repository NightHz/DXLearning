#include "dx9.h"
#include <fstream>
#include <sstream>
#include "Rehenz/noise_gen.h"
#include <algorithm>

namespace Dx9 // contents which not have declaration in header file
{
	struct VertexXYZ
	{
	public:
		float x, y, z;
		static const DWORD FVF = D3DFVF_XYZ;
		VertexXYZ() { x = y = z = 0; }
		VertexXYZ(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
	};

	struct VertexColor
	{
		float x, y, z;
		DWORD color;
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
		static const D3DVERTEXELEMENT9 vertex_color_decl[];
		VertexColor() { x = y = z = 0; color = 0; }
		VertexColor(float _x, float _y, float _z) { x = _x; y = _y; z = _z; color = 0xff000000; }
	};
	const D3DVERTEXELEMENT9 vertex_color_decl[] = {
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
	};

	struct VertexNormal
	{
		float x, y, z;
		float nx, ny, nz;
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL;
		VertexNormal() { x = y = z = 0;  nx = ny = nz = 0; }
		VertexNormal(float _x, float _y, float _z) { x = _x; y = _y; z = _z; nx = ny = nz = 0; }
	};

	struct VertexNormalFaceNormals
	{
		float x, y, z;
		float nx, ny, nz;
		float f1_nx, f1_ny, f1_nz;
		float f2_nx, f2_ny, f2_nz;
		static const D3DVERTEXELEMENT9 vertex_normal_face_normals_decl[];
	};
	// Stream Offset Type Method Usage UsageIndex
	const D3DVERTEXELEMENT9 VertexNormalFaceNormals::vertex_normal_face_normals_decl[] = {
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 1},
		{0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 2},
		D3DDECL_END()
	};

	struct VertexNormalColor
	{
		float x, y, z;
		float nx, ny, nz;
		DWORD color;
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;
		VertexNormalColor() { x = y = z = 0;  nx = ny = nz = 0; color = 0xff000000; }
		VertexNormalColor(float _x, float _y, float _z) { x = _x; y = _y; z = _z; nx = ny = nz = 0; color = 0xff000000; }
	};

	struct VertexNormalColorTex1
	{
		float x, y, z;
		float nx, ny, nz;
		DWORD color;
		float u1, v1;
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
		VertexNormalColorTex1() { x = y = z = 0;  nx = ny = nz = 0; color = 0xff000000; u1 = v1 = 0; }
		VertexNormalColorTex1(float _x, float _y, float _z) { x = _x; y = _y; z = _z; nx = ny = nz = 0; color = 0xff000000; u1 = v1 = 0; }
	};

	void VertexSetVector(WORD offset, void* vertex, D3DXVECTOR3 _vec)
	{
		float* p = reinterpret_cast<float*>(static_cast<BYTE*>(vertex) + offset);
		p[0] = _vec.x;
		p[1] = _vec.y;
		p[2] = _vec.z;
	}

	template <class Vertex>
	void VertexSetNormal(Vertex& v, D3DXVECTOR3 _n)
	{
		v.nx = _n.x;
		v.ny = _n.y;
		v.nz = _n.z;
	}

	template <class Vertex>
	D3DXVECTOR3 VertexToVector(Vertex& v)
	{
		return D3DXVECTOR3(v.x, v.y, v.z);
	}

	template <class Vertex>
	D3DXVECTOR3 TriangleNormal(Vertex& vertex1, Vertex& vertex2, Vertex& vertex3)
	{
		D3DXVECTOR3 p1 = VertexToVector(vertex1), p2 = VertexToVector(vertex2), p3 = VertexToVector(vertex3);
		D3DXVECTOR3 v1 = p2 - p1, v2 = p3 - p1, v3, v4;
		D3DXVec3Cross(&v3, &v1, &v2);
		D3DXVec3Normalize(&v4, &v3);
		return v4;
	}

	template <class Vertex>
	void VertexSetTex1(Vertex& v, float _u, float _v)
	{
		v.u1 = _u;
		v.v1 = _v;
	}

}

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

	DWORD float_to_DWORD(float f)
	{
		return *reinterpret_cast<DWORD*>(&f);
	}

	float DWORD_to_float(DWORD d)
	{
		return *reinterpret_cast<float*>(&d);
	}

	Mesh::Mesh()
	{
		mesh = nullptr;
		subset_count = 0;
		pmesh = nullptr;
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
		if (pmesh)
			pmesh->Release();
		if (vb)
			vb->Release();
		if (ib)
			ib->Release();
	}

	bool Mesh::ComputeBoundingSphere(D3DXVECTOR3& center, float& radius)
	{
		HRESULT hr;
		if (mesh)
		{
			void* vertices;
			hr = mesh->LockVertexBuffer(0, &vertices);
			if (FAILED(hr))
				return false;
			hr = D3DXComputeBoundingSphere(static_cast<D3DXVECTOR3*>(vertices),
				mesh->GetNumVertices(), mesh->GetNumBytesPerVertex(), &center, &radius);
			if (FAILED(hr))
			{
				mesh->UnlockVertexBuffer();
				return false;
			}
			hr = mesh->UnlockVertexBuffer();
			if (FAILED(hr))
				return false;
		}
		else if (vb)
		{
			void* vertices;
			hr = vb->Lock(0, 0, &vertices, 0);
			if (FAILED(hr))
				return false;
			hr = D3DXComputeBoundingSphere(static_cast<D3DXVECTOR3*>(vertices),
				vertex_count, vertex_size, &center, &radius);
			if (FAILED(hr))
			{
				vb->Unlock();
				return false;
			}
			hr = vb->Unlock();
			if (FAILED(hr))
				return false;
		}
		else
			return false;
		return true;
	}

	bool Mesh::ComputeBoundingBox(D3DXVECTOR3& min, D3DXVECTOR3& max)
	{
		HRESULT hr;
		if (mesh)
		{
			void* vertices;
			hr = mesh->LockVertexBuffer(0, &vertices);
			if (FAILED(hr))
				return false;
			hr = D3DXComputeBoundingBox(static_cast<D3DXVECTOR3*>(vertices),
				mesh->GetNumVertices(), mesh->GetNumBytesPerVertex(), &min, &max);
			if (FAILED(hr))
			{
				mesh->UnlockVertexBuffer();
				return false;
			}
			hr = mesh->UnlockVertexBuffer();
			if (FAILED(hr))
				return false;
		}
		else if (vb)
		{
			void* vertices;
			hr = vb->Lock(0, 0, &vertices, 0);
			if (FAILED(hr))
				return false;
			hr = D3DXComputeBoundingBox(static_cast<D3DXVECTOR3*>(vertices),
				vertex_count, vertex_size, &min, &max);
			if (FAILED(hr))
			{
				vb->Unlock();
				return false;
			}
			hr = vb->Unlock();
			if (FAILED(hr))
				return false;
		}
		else
			return false;
		return true;
	}

	bool Mesh::UpdatePMesh()
	{
		if (!mesh)
			return false;
		if (pmesh)
			return true;
		HRESULT hr;

		// generate progressive mesh
		ID3DXBuffer* adjacency_info;
		hr = D3DXCreateBuffer(mesh->GetNumFaces() * 3 * sizeof(DWORD), &adjacency_info);
		if (FAILED(hr))
			return false;
		hr = mesh->GenerateAdjacency(0.001f, static_cast<DWORD*>(adjacency_info->GetBufferPointer()));
		if (FAILED(hr))
		{
			adjacency_info->Release();
			return false;
		}
		hr = D3DXGeneratePMesh(mesh, static_cast<DWORD*>(adjacency_info->GetBufferPointer()),
			nullptr, nullptr, 1, D3DXMESHSIMP_FACE, &pmesh);
		if (FAILED(hr))
		{
			adjacency_info->Release();
			return false;
		}
		adjacency_info->Release();

		// set faces number
		hr = pmesh->SetNumFaces(pmesh->GetMaxFaces());
		if (FAILED(hr))
			return false;

		return true;
	}

	bool Mesh::AdjustProgress(float f)
	{
		if (!pmesh)
			return false;
		HRESULT hr;
		DWORD max = pmesh->GetMaxFaces();
		DWORD min = pmesh->GetMinFaces();
		f = (f > 1 ? 1 : (f < 0 ? 0 : f));
		hr = pmesh->SetNumFaces(min + static_cast<DWORD>(f * (max - min)));
		if (FAILED(hr))
			return false;
		return true;
	}

	bool Mesh::Draw(IDirect3DDevice9* device)
	{
		HRESULT hr;

		if (mesh)
		{
			//draw
			if (!pmesh)
			{
				for (unsigned int i = 0; i < subset_count; i++)
				{
					hr = mesh->DrawSubset(i);
					if (FAILED(hr))
						return false;
				}
			}
			else
			{
				for (unsigned int i = 0; i < subset_count; i++)
				{
					hr = pmesh->DrawSubset(i);
					if (FAILED(hr))
						return false;
				}
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

	std::shared_ptr<Mesh> Mesh::CreateMeshFromFile(IDirect3DDevice9* device, const std::string& file_path)
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

	std::shared_ptr<Mesh> Mesh::CreateMeshNormalFromFile(IDirect3DDevice9* device, const std::string& file_path)
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

	std::shared_ptr<Mesh> Mesh::CreateD3DXCube(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		hr = D3DXCreateBox(device, 1, 1, 1, &mesh->mesh, nullptr);
		if (FAILED(hr))
			return nullptr;
		mesh->subset_count = 1;

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateD3DXSphere(IDirect3DDevice9* device)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		hr = D3DXCreateSphere(device, 1, 20, 20, &mesh->mesh, nullptr);
		if (FAILED(hr))
			return nullptr;
		mesh->subset_count = 1;

		return mesh;
	}

	std::shared_ptr<Mesh> Mesh::CreateTerrainRandom(IDirect3DDevice9* device,
		float terrain_size, int terrain_subdivision, float noise_size, float noise_intensity, unsigned int noise_seed,
		const D3DXCOLOR& low_color, const D3DXCOLOR& high_color)
	{
		auto mesh = std::shared_ptr<Mesh>(new Mesh);
		HRESULT hr;

		// parameters
		const float l = terrain_size;
		const float d = l / 2;
		const int n = terrain_subdivision;
		const int n2 = n * n;
		const float a = l / n;
		Rehenz::PerlinNoise2D noise(noise_seed);
		const float b = noise_size;
		const float f = noise_intensity;

		// create buffer
		mesh->vertex_size = sizeof(VertexColor);
		mesh->fvf = VertexColor::FVF;
		mesh->vertex_count = (n + 1) * (n + 1);
		mesh->index_count = n2 * 2 * 3;
		hr = device->CreateVertexBuffer(mesh->vertex_count * mesh->vertex_size,
			0, mesh->fvf, D3DPOOL_MANAGED, &mesh->vb, 0);
		if (FAILED(hr))
			return nullptr;
		hr = device->CreateIndexBuffer(mesh->index_count * sizeof(WORD),
			D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mesh->ib, 0);
		if (FAILED(hr))
			return nullptr;

		// fill buffer
		VertexColor* vertices;
		WORD* indexes;
		hr = mesh->vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0);
		if (FAILED(hr))
			return nullptr;
		// xyz and color
		for (int iz = 0; iz <= n; iz++)
		{
			for (int ix = 0; ix <= n; ix++)
			{
				int i = iz * (n + 1) + ix;
				float x = ix * a - d;
				float z = iz * a - d;
				float noise_v = noise.GetNoiseSum(x / b, z / b);
				float y = noise_v * f;
				vertices[i] = VertexColor(x, y, z);
				vertices[i].color = low_color + (high_color - low_color) * (noise_v + 1) * 0.5f;
			}
		}
		hr = mesh->vb->Unlock();
		if (FAILED(hr))
			return nullptr;
		hr = mesh->ib->Lock(0, 0, reinterpret_cast<void**>(&indexes), 0);
		if (FAILED(hr))
			return nullptr;
		// indexes
		for (int iz = 0; iz < n; iz++)
		{
			for (int ix = 0; ix < n; ix++)
			{
				int i = (iz * n + ix) * 6;
				WORD v1 = static_cast<WORD>(iz * (n + 1) + ix);
				WORD v2 = v1 + 1;
				WORD v3 = v2 + static_cast<WORD>(n);
				WORD v4 = v3 + 1;
				indexes[i + 0] = v2; indexes[i + 1] = v1; indexes[i + 2] = v3;
				indexes[i + 3] = v2; indexes[i + 4] = v3; indexes[i + 5] = v4;
			}
		}
		hr = mesh->ib->Unlock();
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

	D3DXMATRIX Object::ComputeTransform()
	{
		return ComputeTransform(x, y, z, phi, theta, psi, sx, sy, sz);
	}

	D3DXMATRIX Object::ComputeTransform(float x, float y, float z, float phi, float theta, float psi, float sx, float sy, float sz)
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

	D3DXMATRIX Object::ComputeInverseTransform()
	{
		return ComputeInverseTransform(x, y, z, phi, theta, psi, sx, sy, sz);
	}

	D3DXMATRIX Object::ComputeInverseTransform(float x, float y, float z, float phi, float theta, float psi, float sx, float sy, float sz)
	{
		D3DXMATRIX mat_scale, mat_rotation, mat_position;

		// scale
		D3DXMatrixScaling(&mat_scale, 1 / sx, 1 / sy, 1 / sz);

		// rotation
		D3DXMATRIX mat_phi, mat_theta, mat_psi;
		D3DXMatrixRotationY(&mat_phi, -phi);
		D3DXMatrixRotationZ(&mat_theta, -theta);
		D3DXMatrixRotationY(&mat_psi, -psi);
		mat_rotation = mat_psi * mat_theta * mat_phi;

		// translation
		D3DXMatrixTranslation(&mat_position, -x, -y, -z);

		return mat_position * mat_rotation * mat_scale;
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
		D3DXMatrixRotationY(&mat_yaw, yaw);
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
		D3DXMatrixRotationY(&mat_yaw, yaw);
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
		yaw -= angle;
	}

	void Camera::YawRight(float angle)
	{
		YawLeft(-angle);
	}

	void Camera::PitchUp(float angle)
	{
		pitch -= angle;
	}

	void Camera::PitchDown(float angle)
	{
		PitchUp(-angle);
	}

	D3DXMATRIX Camera::ComputeViewTransform()
	{
		//D3DXMatrixLookAtLH(&mat_view, &pos, &at, &up);
		D3DXMATRIX mat_position, mat_rotation;
		D3DXMatrixTranslation(&mat_position, -pos.x, -pos.y, -pos.z);
		D3DXMATRIX mat_pitch, mat_yaw;
		D3DXMatrixRotationY(&mat_yaw, -yaw);
		D3DXMatrixRotationX(&mat_pitch, -pitch);
		mat_rotation = mat_yaw * mat_pitch;
		return mat_position * mat_rotation;
	}

	D3DXMATRIX Camera::ComputeViewInverseTransform()
	{
		D3DXMATRIX mat_position, mat_rotation;
		D3DXMatrixTranslation(&mat_position, pos.x, pos.y, pos.z);
		D3DXMATRIX mat_pitch, mat_yaw;
		D3DXMatrixRotationY(&mat_yaw, yaw);
		D3DXMatrixRotationX(&mat_pitch, pitch);
		mat_rotation = mat_pitch * mat_yaw;
		D3DXMATRIX mat_view;
		return mat_rotation * mat_position;
	}

	D3DXMATRIX Camera::ComputeProjectionTransform()
	{
		D3DXMATRIX mat_project;
		D3DXMatrixPerspectiveFovLH(&mat_project, fovy, aspect, znear, zfar);
		return mat_project;
	}

	bool Camera::Transform(IDirect3DDevice9* device)
	{
		HRESULT hr;

		// set camera
		D3DXMATRIX mat_view = ComputeViewTransform();
		hr = device->SetTransform(D3DTS_VIEW, &mat_view);
		if (FAILED(hr))
			return false;

		// set projection
		D3DXMATRIX mat_project = ComputeProjectionTransform();
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
		D3DXMATRIX mat_view = mat_reflect * ComputeViewTransform();
		hr = device->SetTransform(D3DTS_VIEW, &mat_view);
		if (FAILED(hr))
			return false;

		// set projection
		D3DXMATRIX mat_project = ComputeProjectionTransform();
		hr = device->SetTransform(D3DTS_PROJECTION, &mat_project);
		if (FAILED(hr))
			return false;

		return true;
	}

	std::pair<bool, float> Camera::IntersectionJudge(const D3DXVECTOR3& ray_o, const D3DXVECTOR3& ray_dir, const D3DXVECTOR3& sphere_c, const float sphere_r)
	{
		// |o + d * t - c| - r = 0;
		// u = o - c
		// d^2 * t^2 + 2*u*d + u^2 - r^2 = 0
		D3DXVECTOR3 u = ray_o - sphere_c;
		float a = D3DXVec3Dot(&ray_dir, &ray_dir);
		float b = 2 * D3DXVec3Dot(&u, &ray_dir);
		float c = D3DXVec3Dot(&u, &u) - sphere_r * sphere_r;
		float delta = b * b - 4 * a * c;
		if (delta < 0)
			return std::make_pair(false, 0.0f);
		else
		{
			float t = (-b - std::sqrtf(delta)) / (2 * a);
			if (t <= 0)
				return std::make_pair(false, 0.0f);
			else
				return std::make_pair(true, t);
		}
	}

	std::pair<bool, float> Camera::IntersectionJudge(const D3DXVECTOR3& ray_o, const D3DXVECTOR3& ray_dir, Object* obj)
	{
		D3DXMATRIX t = obj->ComputeInverseTransform();
		D3DXVECTOR3 ray_o2, ray_dir2, ray_dir3;
		D3DXVec3TransformCoord(&ray_o2, &ray_o, &t);
		D3DXVec3TransformNormal(&ray_dir2, &ray_dir, &t);
		D3DXVec3Normalize(&ray_dir3, &ray_dir2);
		D3DXVECTOR3 sphere_c;
		float sphere_r;
		obj->mesh->ComputeBoundingSphere(sphere_c, sphere_r);
		return IntersectionJudge(ray_o2, ray_dir3, sphere_c, sphere_r);
	}

	std::pair<D3DXVECTOR3, D3DXVECTOR3> Camera::ComputeRay(float x, float y)
	{
		D3DXVECTOR3 ray_o = D3DXVECTOR3(0, 0, 0);
		D3DXVECTOR3 ray_dir = D3DXVECTOR3(0, 0, 1);
		if (x != 0 || y != 0)
		{
			float ymax = std::tanh(fovy * 0.5f);
			ray_dir.y = y * ymax;
			ray_dir.x = x * ymax * aspect;
		}
		D3DXMATRIX t = ComputeViewInverseTransform();
		D3DXVECTOR3 ray_o2, ray_dir2;
		D3DXVec3TransformCoord(&ray_o2, &ray_o, &t);
		D3DXVec3TransformNormal(&ray_dir2, &ray_dir, &t);
		D3DXVec3Normalize(&ray_dir, &ray_dir2);
		return std::make_pair(ray_o2, ray_dir2);
	}

	std::pair<bool, float> Camera::PickObject(float x, float y, Object* obj)
	{
		auto p = ComputeRay(x, y);
		return IntersectionJudge(p.first, p.second, obj);
	}

	std::pair<bool, float> Camera::PickObject(Object* obj)
	{
		return PickObject(0, 0, obj);
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

	bool Texture::SetTexture(IDirect3DDevice9* device)
	{
		HRESULT hr;
		hr = device->SetTexture(0, tex);
		if (FAILED(hr))
			return false;
		return true;
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

	bool Particles::DrawParticles(IDirect3DDevice9* device)
	{
		HRESULT hr;

		hr = device->SetStreamSource(0, vb, 0, vertex_size);
		if (FAILED(hr))
			return false;
		hr = device->SetFVF(fvf);
		if (FAILED(hr))
			return false;

		VertexColor* vertices = nullptr;
		unsigned int particle_in_buffer_count = 0;
		for (const ParticleUnit& p : particles)
		{
			if (!p.is_alive)
				continue;
			if (particle_in_buffer_count == 0)
			{
				// lock when first particle
				hr = vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), D3DLOCK_DISCARD);
				if (FAILED(hr))
					return false;
			}
			// fill buffer
			*vertices = VertexColor(p.pos.x, p.pos.y, p.pos.z);
			vertices->color = p.color + (p.color_fade - p.color) * (p.age / p.life);
			vertices++;
			particle_in_buffer_count++;
			if (particle_in_buffer_count == vertex_count)
			{
				// unlock and draw when buffer is full
				hr = vb->Unlock();
				if (FAILED(hr))
					return false;
				hr = device->DrawPrimitive(D3DPT_POINTLIST, 0, particle_in_buffer_count);
				if (FAILED(hr))
					return false;
				particle_in_buffer_count = 0;
			}
		}
		if (particle_in_buffer_count != 0)
		{
			// unlock and draw when buffer is full
			hr = vb->Unlock();
			if (FAILED(hr))
				return false;
			hr = device->DrawPrimitive(D3DPT_POINTLIST, 0, particle_in_buffer_count);
			if (FAILED(hr))
				return false;
		}

		return true;
	}

	Particles::Particles(IDirect3DDevice9* device, UINT size)
	{
		HRESULT hr;

		// create buffer
		vertex_size = sizeof(VertexColor);
		fvf = VertexColor::FVF;
		vertex_count = size;
		hr = device->CreateVertexBuffer(vertex_count * vertex_size,
			D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS, VertexColor::FVF, D3DPOOL_DEFAULT, &vb, 0);
		if (FAILED(hr))
			vb = nullptr;

		mat.Ambient = D3DXCOLOR(1, 1, 1, 1);
		mat.Diffuse = D3DXCOLOR(1, 1, 1, 1);
		mat.Specular = D3DXCOLOR(1, 1, 1, 1);
		mat.Emissive = D3DXCOLOR(0, 0, 0, 1);
		mat.Power = 8;
		x = y = z = 0;
		phi = theta = psi = 0;
		sx = sy = sz = 1;
		pointsize = 0.05f;
		pointsize_min = 0;
		pointsize_max = 8192;
		pointscale_a = 0;
		pointscale_b = 0;
		pointscale_c = 1.5f;
	}

	Particles::~Particles()
	{
		Die();
	}

	bool Particles::IsAlive()
	{
		if (vb == nullptr)
			return false;
		else
			return true;
	}

	size_t Particles::GetParticlesCount()
	{
		return particles.size();
	}

	bool Particles::Present(unsigned int delta_time)
	{
		for (ParticleUnit& p : particles)
		{
			if (!p.is_alive)
				continue;
			float dt = delta_time / 1000.0f;
			p.age += dt;
			if (p.age > p.life)
			{
				p.is_alive = false;
				continue;
			}
			p.pos += p.vel * dt;
			p.vel += p.acc * dt;
			if (particle_test)
				p.is_alive = particle_test(p);
		}
		particles.erase(std::partition(particles.begin(), particles.end(), [](const auto& p) { return p.is_alive; }), particles.end());

		return true;
	}

	void Particles::Die()
	{
		if (vb != nullptr)
		{
			vb->Release();
			vb = nullptr;
		}
		particles.clear();
	}

	bool Particles::Draw(IDirect3DDevice9* device)
	{
		if (!IsAlive())
			return false;

		HRESULT hr;

		// set point attribute
		hr = device->SetRenderState(D3DRS_POINTSIZE, Dx9::float_to_DWORD(pointsize));
		if (FAILED(hr))
			return false;
		hr = device->SetRenderState(D3DRS_POINTSIZE_MIN, Dx9::float_to_DWORD(pointsize_min));
		if (FAILED(hr))
			return false;
		hr = device->SetRenderState(D3DRS_POINTSIZE_MAX, Dx9::float_to_DWORD(pointsize_max));
		if (FAILED(hr))
			return false;
		hr = device->SetRenderState(D3DRS_POINTSCALE_A, Dx9::float_to_DWORD(pointscale_a));
		if (FAILED(hr))
			return false;
		hr = device->SetRenderState(D3DRS_POINTSCALE_B, Dx9::float_to_DWORD(pointscale_b));
		if (FAILED(hr))
			return false;
		hr = device->SetRenderState(D3DRS_POINTSCALE_C, Dx9::float_to_DWORD(pointscale_c));
		if (FAILED(hr))
			return false;

		// set transform
		D3DXMATRIX mat_world = Object::ComputeTransform(x, y, z, phi, theta, psi, sx, sy, sz);
		hr = device->SetTransform(D3DTS_WORLD, &mat_world);
		if (FAILED(hr))
			return false;

		// set material
		hr = device->SetMaterial(&mat);
		if (FAILED(hr))
			return false;

		// set texture
		if (texture)
		{
			if (!texture->SetTexture(device))
				return false;
		}
		else
		{
			hr = device->SetTexture(0, nullptr);
			if (FAILED(hr))
				return false;
		}

		// draw
		if (!DrawParticles(device))
			return false;

		return true;
	}

	SnowParticles::SnowParticles(IDirect3DDevice9* device, unsigned int seed) : Particles(device), e(seed), d(0, 1)
	{
		pointsize = 0.04f;
		emit_rate = 500;
		range_min = D3DXVECTOR3(-5, -5, -5);
		range_max = D3DXVECTOR3(5, 5, 5);
		vel_min = D3DXVECTOR3(0.1f, -0.4f, 0);
		vel_max = D3DXVECTOR3(0.4f, -1.0f, 0);
	}

	SnowParticles::~SnowParticles()
	{
	}

	void SnowParticles::EmitParticle()
	{
		ParticleUnit p;
		D3DXVECTOR3 range = range_max - range_min;
		p.pos.x = range_min.x + range.x * d(e);
		p.pos.y = range_max.y;
		p.pos.z = range_min.z + range.z * d(e);
		D3DXVECTOR3 vel_range = vel_max - vel_min;
		p.vel.x = vel_min.x + vel_range.x * d(e);
		p.vel.y = vel_min.y + vel_range.y * d(e);
		p.vel.z = vel_min.z + vel_range.z * d(e);
		p.acc = D3DXVECTOR3(0, 0, 0);
		p.life = 100;
		p.age = 0;
		p.color = D3DXCOLOR(1, 1, 1, 1);
		p.color_fade = D3DXCOLOR(1, 1, 1, 1);
		p.is_alive = true;
		particles.push_back(p);
	}

	bool SnowParticles::Present(unsigned int delta_time)
	{
		particle_test = [this](const ParticleUnit& p)
		{
			return p.pos.x >= this->range_min.x && p.pos.y >= this->range_min.y && p.pos.z >= this->range_min.z &&
				p.pos.x <= this->range_max.x && p.pos.y <= this->range_max.y && p.pos.z <= this->range_max.z;
		};
		Particles::Present(delta_time);
		int emit_count = emit_rate * delta_time / 1000;
		for (int i = 0; i < emit_count; i++)
			EmitParticle();
		return true;
	}

	FireworkParticles::FireworkParticles(IDirect3DDevice9* device, unsigned int seed) : Particles(device), e(seed), d(0, 1)
	{
		pointsize = 0.03f;
		emit_rate = 1000;
		radius = 0.1f;
		vel_min = 0.1f;
		vel_max = 0.4f;
		life = 2.0f;
	}

	FireworkParticles::~FireworkParticles()
	{
	}

	void FireworkParticles::EmitParticle()
	{
		ParticleUnit p;
		do
		{
			p.pos.x = d(e) * 2 - 1;
			p.pos.y = d(e) * 2 - 1;
			p.pos.z = d(e) * 2 - 1;
		} while (D3DXVec3LengthSq(&p.pos) >= 1.0f);
		p.pos *= radius;
		do
		{
			p.vel.x = d(e) * 2 - 1;
			p.vel.y = d(e) * 2 - 1;
			p.vel.z = d(e) * 2 - 1;
		} while (D3DXVec3LengthSq(&p.vel) >= 1.0f && D3DXVec3LengthSq(&p.vel) <= 0.01f);
		D3DXVECTOR3 vel;
		D3DXVec3Normalize(&vel, &p.vel);
		p.vel = vel * (d(e) * (vel_max - vel_min) + vel_min);
		p.acc = D3DXVECTOR3(0, 0, 0);
		p.life = life;
		p.age = 0;
		p.color.a = 1;
		p.color.r = d(e);
		p.color.g = d(e);
		p.color.b = d(e);
		p.color_fade = p.color;
		p.is_alive = true;
		particles.push_back(p);
	}

	bool FireworkParticles::Present(unsigned int delta_time)
	{
		Particles::Present(delta_time);
		int emit_count = emit_rate * delta_time / 1000;
		for (int i = 0; i < emit_count; i++)
			EmitParticle();
		return true;
	}

	GunParticles::GunParticles(IDirect3DDevice9* device, unsigned int seed) : Particles(device), e(seed), d(0, 1)
	{
		pointsize = 0.01f;
		emit_rate = 1000;
		dir = D3DXVECTOR3(1, 0, 0);
		theta = D3DX_PI / 180.0f * 30.0f;
		max_radius = 2.0f;
		vel_min = 0.1f;
		vel_max = 1.0f;
		color = D3DXCOLOR(1, 0.5f, 0.5f, 1);
	}

	GunParticles::~GunParticles()
	{
	}

	void GunParticles::EmitParticle()
	{
		float cos_angle = min(std::cosf(D3DX_PI / 180.0f * 5.0f), std::cosf(theta * 0.5f));
		ParticleUnit p;
		p.pos = D3DXVECTOR3(0, 0, 0);
		while (true)
		{
			p.vel.x = d(e) * 2 - 1;
			p.vel.y = d(e) * 2 - 1;
			p.vel.z = d(e) * 2 - 1;
			if (D3DXVec3LengthSq(&p.vel) >= 1.0f && D3DXVec3LengthSq(&p.vel) <= 0.01f)
				continue;
			D3DXVECTOR3 vel;
			D3DXVec3Normalize(&vel, &p.vel);
			if (D3DXVec3Dot(&vel, &dir) >= cos_angle)
			{
				p.vel = vel * (d(e) * (vel_max - vel_min) + vel_min);
				break;
			}
		};
		p.acc = D3DXVECTOR3(0, 0, 0);
		p.life = 100;
		p.age = 0;
		p.color = color;
		p.color_fade = color;
		p.is_alive = true;
		particles.push_back(p);
	}

	bool GunParticles::Present(unsigned int delta_time)
	{
		float r2 = max_radius * max_radius;
		particle_test = [r2](const ParticleUnit& p)
		{
			return D3DXVec3LengthSq(&p.pos) <= r2;
		};
		Particles::Present(delta_time);
		int emit_count = emit_rate * delta_time / 1000;
		for (int i = 0; i < emit_count; i++)
			EmitParticle();
		return true;
	}

	VertexShader::VertexShader(IDirect3DDevice9* device, const std::string& file)
	{
		vs = nullptr;
		ct = nullptr;

		HRESULT hr = 0;
		// compile shader file
		ID3DXBuffer* buffer, * error_buffer = nullptr;
		hr = D3DXCompileShaderFromFile(file.c_str(), nullptr, nullptr, "main", "vs_2_0", D3DXSHADER_DEBUG, &buffer, &error_buffer, &ct);
		if (error_buffer != nullptr)
		{
			error_buffer->Release();
			return;
		}
		if (FAILED(hr))
			return;

		// create vertex shader
		hr = device->CreateVertexShader(static_cast<DWORD*>(buffer->GetBufferPointer()), &vs);
		if (FAILED(hr))
		{
			buffer->Release();
			vs->Release();
			vs = nullptr;
			return;
		}
		buffer->Release();
	}

	VertexShader::~VertexShader()
	{
		if (vs)
			vs->Release();
		if (ct)
			ct->Release();
	}

	VertexShader::operator bool()
	{
		return vs != nullptr;
	}

	bool VertexShader::Enable(IDirect3DDevice9* device)
	{
		HRESULT hr = device->SetVertexShader(vs);
		if (FAILED(hr))
			return false;
		return true;
	}

}
