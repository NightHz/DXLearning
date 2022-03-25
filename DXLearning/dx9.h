#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#include <d3dx9.h>
#include "window.h"
#include <memory>
#include <vector>
#include <random>
#include <functional>

namespace Dx9
{
	IDirect3DDevice9* CreateSimpleDx9Device(SimpleWindow* window);

	DWORD float_to_DWORD(float f);
	float DWORD_to_float(DWORD d);

	class Mesh
	{
	private:
		ID3DXMesh* mesh;
		UINT subset_count;
		ID3DXPMesh* pmesh;

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

		bool ComputeBoundingSphere(D3DXVECTOR3& center, float& radius);
		bool ComputeBoundingBox(D3DXVECTOR3& min, D3DXVECTOR3& max);
		bool UpdatePMesh();
		bool AdjustProgress(float f);

		bool Draw(IDirect3DDevice9* device);

		static std::shared_ptr<Mesh> CreateCubeXYZ(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateCubeColor(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateTetrahedronNormal(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateTetrahedronNormalColor(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateCubeNormalColorTex1(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreatePlaneNormal(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateD3DXTeapot(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateD3DXText(IDirect3DDevice9* device, const std::string& text);
		static std::shared_ptr<Mesh> CreateMeshFromFile(IDirect3DDevice9* device, const std::string& file_path);
		static std::shared_ptr<Mesh> CreateMeshNormalFromFile(IDirect3DDevice9* device, const std::string& file_path);
		static std::shared_ptr<Mesh> CreateD3DXCube(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateD3DXSphere(IDirect3DDevice9* device);
		static std::shared_ptr<Mesh> CreateTerrainRandom(IDirect3DDevice9* device,
			float terrain_size = 20.0f, int terrain_subdivision = 100,
			float noise_size = 8.0f, float noise_intensity = 6.0f, unsigned int noise_seed = 7364852u,
			const D3DXCOLOR& low_color = D3DXCOLOR(1, 1, 1, 1), const D3DXCOLOR& high_color = D3DXCOLOR(0, 0, 0, 1));
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

		bool SetTexture(IDirect3DDevice9* device);

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
		static D3DXMATRIX ComputeTransform(float x, float y, float z, float phi, float theta, float psi, float sx, float sy, float sz);
		D3DXMATRIX ComputeInverseTransform();
		static D3DXMATRIX ComputeInverseTransform(float x, float y, float z, float phi, float theta, float psi, float sx, float sy, float sz);

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

		D3DXMATRIX ComputeViewTransform();
		D3DXMATRIX ComputeViewInverseTransform();
		D3DXMATRIX ComputeProjectionTransform();

		bool Transform(IDirect3DDevice9* device);
		bool TransformReflect(IDirect3DDevice9* device, const D3DXPLANE& plane);

		static std::pair<bool, float> IntersectionJudge(const D3DXVECTOR3& ray_o, const D3DXVECTOR3& ray_dir, const D3DXVECTOR3& sphere_c, const float sphere_r);
		static std::pair<bool, float> IntersectionJudge(const D3DXVECTOR3& ray_o, const D3DXVECTOR3& ray_dir, Object* obj);
		std::pair<D3DXVECTOR3, D3DXVECTOR3> ComputeRay(float x, float y);
		std::pair<bool, float> PickObject(float x, float y, Object* obj);
		std::pair<bool, float> PickObject(Object* obj);
	};

	class Particles
	{
	private:
		IDirect3DVertexBuffer9* vb;
		UINT vertex_count;
		UINT vertex_size;
		DWORD fvf;

		bool DrawParticles(IDirect3DDevice9* device);

	protected:
		struct ParticleUnit
		{
			D3DXVECTOR3 pos;
			D3DXVECTOR3 vel;
			D3DXVECTOR3 acc;
			float life;
			float age;
			D3DXCOLOR color;
			D3DXCOLOR color_fade;
			bool is_alive;
			ParticleUnit() : pos(), vel(), acc(), color(), color_fade()
			{
				life = age = 0;
				is_alive = false;
			}
		};
		std::vector<ParticleUnit> particles;
		std::function<bool(const ParticleUnit&)> particle_test;

	public:
		std::shared_ptr<Texture> texture;

		D3DMATERIAL9 mat;

		float x, y, z;
		float phi, theta, psi;
		float sx, sy, sz;

		float pointsize;
		float pointsize_min, pointsize_max;
		float pointscale_a, pointscale_b, pointscale_c;

		Particles(IDirect3DDevice9* device, UINT size = 500);
		Particles(const Particles&) = delete;
		Particles& operator=(const Particles&) = delete;
		~Particles();

		bool IsAlive();
		size_t GetParticlesCount();

		// only update existed particles
		virtual bool Present(unsigned int delta_time);
		void Die();

		bool Draw(IDirect3DDevice9* device);
	};

	class SnowParticles : public Particles
	{
	private:
		std::default_random_engine e;
		std::uniform_real_distribution<float> d;

	public:
		int emit_rate;
		D3DXVECTOR3 range_min, range_max;
		D3DXVECTOR3 vel_min, vel_max;

		SnowParticles(IDirect3DDevice9* device, unsigned int seed = 8734652u);
		SnowParticles(const SnowParticles&) = delete;
		SnowParticles& operator=(const SnowParticles&) = delete;
		~SnowParticles();

		void EmitParticle();
		virtual bool Present(unsigned int delta_time) override;
	};

	class FireworkParticles : public Particles
	{
	private:
		std::default_random_engine e;
		std::uniform_real_distribution<float> d;

	public:
		int emit_rate;
		float radius;
		float vel_min, vel_max;
		float life;

		FireworkParticles(IDirect3DDevice9* device, unsigned int seed = 8734652u);
		FireworkParticles(const FireworkParticles&) = delete;
		FireworkParticles& operator=(const FireworkParticles&) = delete;
		~FireworkParticles();

		void EmitParticle();
		virtual bool Present(unsigned int delta_time) override;
	};

	class GunParticles : public Particles
	{
	private:
		std::default_random_engine e;
		std::uniform_real_distribution<float> d;

	public:
		int emit_rate;
		D3DXVECTOR3 dir;
		float theta;
		float max_radius;
		float vel_min, vel_max;
		D3DXCOLOR color;

		GunParticles(IDirect3DDevice9* device, unsigned int seed = 8734652u);
		GunParticles(const GunParticles&) = delete;
		GunParticles& operator=(const GunParticles&) = delete;
		~GunParticles();

		void EmitParticle();
		virtual bool Present(unsigned int delta_time) override;
	};

	class VertexShader
	{
	private:
		IDirect3DVertexShader9* vs;
		ID3DXConstantTable* ct;

	public:
		VertexShader(IDirect3DDevice9* device, const std::string& file);
		VertexShader(const VertexShader&) = delete;
		VertexShader& operator=(const VertexShader&) = delete;
		~VertexShader();

		operator bool();

		ID3DXConstantTable* GetCT() { return ct; }
		bool Enable(IDirect3DDevice9* device);
	};
}
