#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#include <d3dx9.h>
#include "window.h"
#include <memory>

namespace Dx9
{
	IDirect3DDevice9* CreateSimpleDx9Device(SimpleWindow* window);

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
		VertexColor() { x = y = z = 0; color = 0; }
		VertexColor(float _x, float _y, float _z) { x = _x; y = _y; z = _z; color = 0xff000000; }
	};

	struct VertexNormal
	{
		float x, y, z;
		float nx, ny, nz;
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL;
		VertexNormal() { x = y = z = 0;  nx = ny = nz = 0; }
		VertexNormal(float _x, float _y, float _z) { x = _x; y = _y; z = _z; nx = ny = nz = 0; }
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

	class Mesh
	{
	private:
		ID3DXMesh* mesh;
		UINT subset_count;
		IDirect3DVertexBuffer9* vb;
		IDirect3DIndexBuffer9* ib;
		UINT vertex_count;
		UINT vertex_size;
		DWORD fvf;
		UINT index_count;

		Mesh();

	public:
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		~Mesh();

		bool Draw(IDirect3DDevice9* device);

		static std::shared_ptr<Mesh> CreateCubeXYZ(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateCubeColor(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateTetrahedronNormal(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateTetrahedronNormalColor(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateCubeNormalColorTex1(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreatePlaneNormal(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateD3DXTeapot(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateD3DXText(IDirect3DDevice9* device, const std::string& text);
		static std::shared_ptr<Mesh> CreateFromFile(IDirect3DDevice9* device, const std::string& file_path);
		static std::shared_ptr<Mesh> CreateFromFileNormal(IDirect3DDevice9* device, const std::string& file_path);
	};

	class Texture
	{
		friend class Object;
	private:
		IDirect3DTexture9* tex;

		Texture();

	public:
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		~Texture();

		static std::shared_ptr<Texture> CreateTexture(IDirect3DDevice9* device, const char* filename);
	};

	class Object
	{
	private:
	public:
		std::shared_ptr<Mesh> mesh;

		std::shared_ptr<Texture> texture;

		D3DMATERIAL9 mat;

		float x, y, z;
		float phi, theta, psi;
		float sx, sy, sz;

		Object(std::shared_ptr<Mesh> _mesh = nullptr);
		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		~Object();

		D3DXMATRIX ComputeTransform();

		bool Draw(IDirect3DDevice9* device);
		bool DrawMirror(IDirect3DDevice9* device, const D3DXPLANE& plane);
		bool DrawShadow(IDirect3DDevice9* device, const D3DXVECTOR4& light_dir, const D3DXPLANE& plane);
	};

	class Camera
	{
	private:

	public:
		D3DXVECTOR3 pos;
		float pitch, yaw;
		float fovy, aspect, znear, zfar;

		Camera();
		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;
		~Camera();

		void MoveFront(float d);
		void MoveBack(float d);
		void MoveLeft(float d);
		void MoveRight(float d);
		void MoveUp(float d);
		void MoveDown(float d);

		void YawLeft(float angle);
		void YawRight(float angle);
		void PitchUp(float angle);
		void PitchDown(float angle);

		bool Transform(IDirect3DDevice9* device);
		bool TransformReflect(IDirect3DDevice9* device, const D3DXPLANE& plane);
	};
}
