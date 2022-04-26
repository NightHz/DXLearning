#pragma once
#include "math.h"
#include <vector>
#include <memory>
#include "type.h"

namespace Rehenz
{
	// 4-component color, r=x, b=y, g=z, a=w, defualt a=1
	typedef Point Color;
	// 2-component uv coordinates, u=x, v=y
	typedef Vector2 UV;

	struct Vertex;
	class Mesh;
	class Texture;

	struct VertexDistance;



	struct Vertex
	{
	public:
		// position
		Point p;
		// color
		Color c;
		UV uv;
		UV uv2;
		// default value is 1, save multiplication factor
		// when multiply by 1/w to standardize p, save 1/w
		float coef;

		Vertex(Point _p);
		explicit Vertex(Point _p, Color _c, UV _uv = UV(0, 0), UV _uv2 = UV(0, 0), float _rhw = 1);
		explicit Vertex(Point _p, UV _uv, UV _uv2 = UV(0, 0));

		Vertex& operator*=(float);
		Vertex& operator+=(const Vertex&);

		Vertex operator*(float) const;
		Vertex operator+(const Vertex&) const;
		Vertex operator-(const Vertex&) const;
	};

	void VertexNormalize(Vertex& v);
	Vertex VertexLerp(const Vertex& v1, const Vertex& v2, float t);

	class Mesh
	{
	private:
		std::vector<Vertex> vertices;
		std::vector<int> triangles;

	public:
		Mesh();
		explicit Mesh(const std::vector<Vertex>& _vertices, const std::vector<int>& _triangles);
		~Mesh();

		size_t VertexCount() { return vertices.size(); }
		size_t TriangleCount() { return triangles.size() / 3; }
		const std::vector<Vertex>& GetVertices() { return vertices; }
		const std::vector<int>& GetTriangles() { return triangles; }

		void AddVertex(Vertex vertex);
		void AddVertex(const std::vector<Vertex>& _vertices);
		void AddTriangle(int a, int b, int c);
		void AddTriangle(const std::vector<int>& _triangles);
	};

	// create cube mesh which includes pos, color, uv, uv2 info
	// pos   : unit cube, and (0,0,0) is center point
	// color : use colors fill, and default is white
	// uv    : each faces map to the whole texture
	// uv2   : expanded faces map to the texture
	std::shared_ptr<Mesh> CreateCubeMesh(const std::vector<Color>& colors = std::vector<Color>());

	// create sphere mesh which includes pos info
	// pos   : unit sphere, and (0,0,0) is center point
	std::shared_ptr<Mesh> CreateSphereMesh(int smooth = 10);
	// another sphere mesh layout. I call it B layout. default is A layout
	std::shared_ptr<Mesh> CreateSphereMeshB(int smooth = 5);
	// sphere mesh C layout
	std::shared_ptr<Mesh> CreateSphereMeshC(int smooth = 8);
	// sphere mesh D layout
	std::shared_ptr<Mesh> CreateSphereMeshD(int smooth = 4);

	class Texture
	{
	private:
	public:
		const int width;
		const int height;
		Color* const buffer;

		Texture(int _width, int _height);
		Texture(const Texture& t);
		Texture& operator=(const Texture&) = delete;
		~Texture();

		Color GetColor(float u, float v);
		Color GetColor(UV uv);
	};

	// create digit 1 texture
	std::shared_ptr<Texture> CreateTexture1();
	// create letter C texture
	std::shared_ptr<Texture> CreateTextureC();
	// create white-blue plaid texture
	std::shared_ptr<Texture> CreateTexturePlaid();
	// create 1-6 dice texture
	std::shared_ptr<Texture> CreateTextureDice();
}