#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#include <d3dx9.h>
#include "window.h"
#include <memory>

namespace Dx9
{
	IDirect3DDevice9* CreateSimpleDx9Device(SimpleWindow* window);

	struct Vertex
	{
	public:
		float x, y, z;
		static const DWORD FVF = D3DFVF_XYZ;
		Vertex() { x = y = z = 0; }
		Vertex(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
	};

	struct ColorVertex
	{
		float x, y, z;
		DWORD color;
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
		ColorVertex() { x = y = z = 0; color = 0; }
		ColorVertex(float _x, float _y, float _z, DWORD _color) { x = _x; y = _y; z = _z; color = _color; }
	};

	struct NormalTexVertex
	{
		float x, y, z;
		float nx, ny, nz;
		float u, v;
		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
		NormalTexVertex() { x = y = z = 0;  nx = ny = nz = 0; u = v = 0; }
		NormalTexVertex(float _x, float _y, float _z, float _nx, float _ny, float _nz, float _u, float _v)
		{
			x = _x; y = _y; z = _z;
			nx = _nx; ny = _ny; nz = _nz;
			u = _u; v = _v;
		}
	};

	class Object
	{
	private:
		Object();

	public:
		IDirect3DVertexBuffer9* vb;
		IDirect3DIndexBuffer9* ib;

		float x, y, z;
		float phi, theta, psi;
		float sx, sy, sz;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		~Object();

		bool Transform(IDirect3DDevice9* device);
		bool Draw(IDirect3DDevice9* device);

		static std::shared_ptr<Object> CreateCube(IDirect3DDevice9* device);
	};

	class Camera
	{
	private:

	public:
		D3DXVECTOR3 pos;
		D3DXVECTOR3 at;
		D3DXVECTOR3 up;
		float fovy, aspect, znear, zfar;

		Camera();
		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;
		~Camera();

		bool Transform(IDirect3DDevice9* device);

		static std::shared_ptr<Camera> CreateCamera(SimpleWindow* window);
	};
}
