#pragma once
#include "type.h"
#include "math.h"
#include <vector>
#include <memory>
#include "mesh.h"

namespace Rehenz
{
	class Transform;
	class Projection;

	class RenderObject;
	class RenderScene;

	class Camera;



	class Transform
	{
	public:
		Vector pos;
		AircraftAxes axes;
		Vector scale;

		Transform();
		~Transform();

		Matrix GetTransformMatrix();
		Matrix GetInverseTransformMatrix();

		Vector GetFront();
		Vector GetUp();
		Vector GetRight();
		Vector GetFrontInGround();
		Vector GetRightInGround();

		void SetFront(Vector front);
	};

	class Projection
	{
	public:
		float fovy, aspect, z_near, z_far;

		// default fovy = pi/2, aspect = 1, z_near = 1, z_far = 500
		Projection();
		~Projection();

		Matrix GetTransformMatrix();

		Point GetOrigin();
	};

	class RenderObject
	{
	private:
	public:
		Transform transform;

		std::shared_ptr<Mesh> pmesh;

		std::shared_ptr<Texture> texture;
		std::shared_ptr<Texture> texture2;

		explicit RenderObject(std::shared_ptr<Mesh> _pmesh = nullptr,
			std::shared_ptr<Texture> _pt = nullptr, std::shared_ptr<Texture> _pt2 = nullptr);
		~RenderObject();
	};

	class RenderScene
	{
	private:
		// save all objects to render
		std::vector<std::shared_ptr<RenderObject>> objs;

	public:
		// reader for obj
		class obj_reader
		{
			friend class RenderScene;
		private:
			int index;
			RenderObject* pobj;
		public:
			obj_reader() { index = 0; pobj = nullptr; }
			inline operator bool() { return pobj != nullptr; }
			inline RenderObject* operator->() { return pobj; }
		};

		RenderScene();
		~RenderScene();

		void AddRenderObject(std::shared_ptr<RenderObject> pobj);
		bool RemoveRenderObject(std::shared_ptr<RenderObject> pobj);

		// get first object
		obj_reader GetRenderObject();
		// get next object, and next of last object is false
		obj_reader GetRenderObject(obj_reader prev);

		static RenderScene global_scene;
	};

	inline void AddRenderObject(std::shared_ptr<RenderObject> pobj)
	{
		RenderScene::global_scene.AddRenderObject(pobj);
	}

	inline bool RemoveRenderObject(std::shared_ptr<RenderObject> pobj)
	{
		RenderScene::global_scene.RemoveRenderObject(pobj);
	}

	// get first object
	inline RenderScene::obj_reader GetRenderObject()
	{
		RenderScene::global_scene.GetRenderObject();
	}

	// get next object, and next of last object is false
	inline RenderScene::obj_reader GetRenderObject(RenderScene::obj_reader prev)
	{
		RenderScene::global_scene.GetRenderObject(prev);
	}

	class Camera
	{
	private:
		int height, width;
		// last buffer image
		uint* buffer;

	public:
		Transform transform;
		Projection projection;

		enum class RenderMode { Wireframe, PureWhite, /*FlatColor,*/ Shader };
		RenderMode render_mode;
		VertexShader vertex_shader;
		PixelShader pixel_shader;

		// default pos = (0,0,-5)
		explicit Camera(int _height, int _width);
		Camera(const Camera& c);
		Camera& operator=(const Camera& c) = delete;
		~Camera();

		inline int GetHeight() { return height; }
		inline int GetWidth() { return width; }
		inline const uint* GetLastImage() { return buffer; }

		void SetSize(int _height, int _width);

		const uint* RenderImage(RenderScene& scene);
		inline const uint* RenderImage()
		{
			return RenderImage(RenderScene::global_scene);
		}
	};
}