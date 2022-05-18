#include "mesh.h"
#include "util.h"
#include <fstream>
#include <sstream>

namespace Rehenz
{
	VertexShader DefaultVertexShader = [](const VertexShaderData& data, const Vertex& v0)->Vertex
	{
		Vertex v(v0);
		v.p = v.p * data.transform;
		return v;
	};

	PixelShader DefaultPixelShader = [](const PixelShaderData& data, const Vertex& v0)->Color
	{
		(data); // unreferenced
		return v0.c;
	};

	PixelShader TexturePixelShader = [](const PixelShaderData& data, const Vertex& v0)->Color
	{
		if (data.texture != nullptr)
			return data.texture->GetColor(v0.uv);
		else
			return v0.c;
	};

	Vertex::Vertex(Point _p) : p(_p), n(0, 0, 0), c(1, 1, 1), uv(0, 0), uv2(0, 0), coef(1)
	{
	}

	Vertex::Vertex(Point _p, Vector _n, Color _c, UV _uv, UV _uv2, float _coef)
		: p(_p), n(_n), c(_c), uv(_uv), uv2(_uv2), coef(_coef)
	{
	}

	Vertex::Vertex(Point _p, Color _c, UV _uv, UV _uv2, float _coef)
		: p(_p), n(0, 0, 0), c(_c), uv(_uv), uv2(_uv2), coef(_coef)
	{
	}

	Vertex::Vertex(Point _p, UV _uv, UV _uv2, float _coef)
		: p(_p), n(0, 0, 0), c(1, 1, 1), uv(_uv), uv2(_uv2), coef(_coef)
	{
	}

	Vertex& Vertex::operator*=(float f)
	{
		p *= f;
		n *= f;
		c *= f;
		uv *= f;
		uv2 *= f;
		coef *= f;
		return *this;
	}

	Vertex& Vertex::operator+=(const Vertex& v)
	{
		p += v.p;
		n += v.n;
		c += v.c;
		uv += v.uv;
		uv2 += v.uv2;
		coef += v.coef;
		return *this;
	}

	Vertex Vertex::operator*(float f) const
	{
		return Vertex(p * f, n * f, c * f, uv * f, uv2 * f, coef * f);
	}

	Vertex Vertex::operator+(const Vertex& v) const
	{
		return Vertex(p + v.p, n + v.n, c + v.c, uv + v.uv, uv2 + v.uv2, coef + v.coef);
	}

	Vertex Vertex::operator-(const Vertex& v) const
	{
		return Vertex(p - v.p, n - v.n, c - v.c, uv - v.uv, uv2 - v.uv2, coef - v.coef);
	}

	Color operator*(Color c1, Color c2)
	{
		return Color(c1.x * c2.x, c1.y * c2.y, c1.z * c2.z, c1.w * c2.w);
	}

	Vertex VertexRecover(const Vertex& v)
	{
		float f = 1 / v.coef;
		return Vertex(v.p, v.n * f, v.c * f, v.uv * f, v.uv2 * f, 1);
	}

	Vertex VertexLerp(const Vertex& v1, const Vertex& v2, float t)
	{
		return v1 + (v2 - v1) * t;
	}



	Mesh::Mesh() : vertices(), triangles()
	{
	}
	Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<int>& _triangles) : vertices(_vertices), triangles(_triangles)
	{
	}
	Mesh::Mesh(const std::vector<Vertex>&& _vertices, const std::vector<int>&& _triangles) : vertices(_vertices), triangles(_triangles)
	{
	}
	Mesh::~Mesh()
	{
	}
	void Mesh::AddVertex(Vertex vertex)
	{
		vertices.push_back(vertex);
	}
	void Mesh::AddVertex(const std::vector<Vertex>& _vertices)
	{
		vertices.insert(vertices.end(), _vertices.begin(), _vertices.end());
	}
	void Mesh::AddTriangle(int a, int b, int c)
	{
		triangles.push_back(a);
		triangles.push_back(b);
		triangles.push_back(c);
	}
	void Mesh::AddTriangle(const std::vector<int>& _triangles)
	{
		triangles.insert(triangles.end(), _triangles.begin(), _triangles.end());
	}



	std::shared_ptr<Mesh> CreateCubeMesh(const std::vector<Color>& colors)
	{
		std::vector<Vertex> vertices;
		std::vector<int> triangles;

		auto GetColor = [&colors](int i) -> Color
		{
			if (colors.size() == 0)
				return Color(1, 1, 1);
			else
				return colors[i % colors.size()];
		};
		// front
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, -0.5f), Vector(0, 0, -1), GetColor(0), UV(0, 1), UV(0.25f, 0.5f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, -0.5f), Vector(0, 0, -1), GetColor(1), UV(1, 1), UV(0.5f, 0.5f)));
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, -0.5f), Vector(0, 0, -1), GetColor(2), UV(0, 0), UV(0.25f, 0.25f)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, -0.5f), Vector(0, 0, -1), GetColor(3), UV(1, 0), UV(0.5f, 0.25f)));
		triangles.push_back(0); triangles.push_back(2); triangles.push_back(1);
		triangles.push_back(2); triangles.push_back(3); triangles.push_back(1);
		// down
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, 0.5f), Vector(0, -1, 0), GetColor(4), UV(0, 1), UV(0.25f, 0.75f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, 0.5f), Vector(0, -1, 0), GetColor(5), UV(1, 1), UV(0.5f, 0.75f)));
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, -0.5f), Vector(0, -1, 0), GetColor(0), UV(0, 0), UV(0.25f, 0.5f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, -0.5f), Vector(0, -1, 0), GetColor(1), UV(1, 0), UV(0.5f, 0.5f)));
		triangles.push_back(4); triangles.push_back(6); triangles.push_back(5);
		triangles.push_back(6); triangles.push_back(7); triangles.push_back(5);
		// up
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, -0.5f), Vector(0, 1, 0), GetColor(2), UV(0, 1), UV(0.25f, 0.25f)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, -0.5f), Vector(0, 1, 0), GetColor(3), UV(1, 1), UV(0.5f, 0.25f)));
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, 0.5f), Vector(0, 1, 0), GetColor(6), UV(0, 0), UV(0.25f, 0)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, 0.5f), Vector(0, 1, 0), GetColor(7), UV(1, 0), UV(0.5f, 0)));
		triangles.push_back(8); triangles.push_back(10); triangles.push_back(9);
		triangles.push_back(10); triangles.push_back(11); triangles.push_back(9);
		// back
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, 0.5f), Vector(0, 0, 1), GetColor(6), UV(0, 1), UV(0.25f, 1)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, 0.5f), Vector(0, 0, 1), GetColor(7), UV(1, 1), UV(0.5f, 1)));
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, 0.5f), Vector(0, 0, 1), GetColor(4), UV(0, 0), UV(0.25f, 0.75f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, 0.5f), Vector(0, 0, 1), GetColor(5), UV(1, 0), UV(0.5f, 0.75f)));
		triangles.push_back(12); triangles.push_back(14); triangles.push_back(13);
		triangles.push_back(14); triangles.push_back(15); triangles.push_back(13);
		// left
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, 0.5f), Vector(-1, 0, 0), GetColor(4), UV(0, 1), UV(0, 0.5f)));
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, -0.5f), Vector(-1, 0, 0), GetColor(0), UV(1, 1), UV(0.25f, 0.5f)));
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, 0.5f), Vector(-1, 0, 0), GetColor(6), UV(0, 0), UV(0, 0.25f)));
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, -0.5f), Vector(-1, 0, 0), GetColor(2), UV(1, 0), UV(0.25f, 0.25f)));
		triangles.push_back(16); triangles.push_back(18); triangles.push_back(17);
		triangles.push_back(18); triangles.push_back(19); triangles.push_back(17);
		// right
		vertices.push_back(Vertex(Point(0.5f, -0.5f, -0.5f), Vector(1, 0, 0), GetColor(1), UV(0, 1), UV(0.5f, 0.5f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, 0.5f), Vector(1, 0, 0), GetColor(5), UV(1, 1), UV(0.75f, 0.5f)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, -0.5f), Vector(1, 0, 0), GetColor(3), UV(0, 0), UV(0.5f, 0.25f)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, 0.5f), Vector(1, 0, 0), GetColor(7), UV(1, 0), UV(0.75f, 0.25f)));
		triangles.push_back(20); triangles.push_back(22); triangles.push_back(21);
		triangles.push_back(22); triangles.push_back(23); triangles.push_back(21);

		return std::make_shared<Mesh>(std::move(vertices), std::move(triangles));
	}
	std::shared_ptr<Mesh> CreateSphereMesh(int smooth)
	{
		smooth = Clamp(smooth, 2, 200);
		int yn = smooth, xn = 2 * smooth;

		std::vector<Vertex> vertices;
		std::vector<int> triangles;

		// add vertices by latitude from top(0,1,0) to bottom(0,-1,0)
		vertices.emplace_back(Point(0, 1, 0), Vector(0, 1, 0));
		for (int y = 1; y < yn; y++)
		{
			float theta = pi * y / yn;
			for (int x = 0; x < xn; x++)
			{
				float phi = pi_mul2 * x / xn;
				vertices.push_back(Point(sinf(theta) * cosf(phi), cosf(theta), -sinf(theta) * sinf(phi)));
				vertices.back().n = vertices.back().p;
				vertices.back().n.w = 0;
			}
		}
		vertices.push_back(Point(0, -1, 0));

		// add first part triangles, which include top vertex
		for (int i = 1; i < xn; i++)
		{
			triangles.push_back(0); triangles.push_back(i); triangles.push_back(i + 1);
		}
		triangles.push_back(0); triangles.push_back(xn); triangles.push_back(1);
		// add second part triangles
		for (int j = 2; j < yn; j++)
		{
			int a = 1 + xn * (j - 2); // upper starting vertex
			int b = 1 + xn * (j - 1); // below starting vertex
			for (int i = 0; i < xn - 1; i++)
			{
				triangles.push_back(a + i); triangles.push_back(b + i); triangles.push_back(b + i + 1);
				triangles.push_back(a + i); triangles.push_back(b + i + 1); triangles.push_back(a + i + 1);
			}
			triangles.push_back(a + xn - 1); triangles.push_back(b + xn - 1); triangles.push_back(b);
			triangles.push_back(a + xn - 1); triangles.push_back(b); triangles.push_back(a);
		}
		// add last part triangles, which include bottom vertex
		int bottom = xn * (yn - 1) + 1;
		int a = 1 + xn * (yn - 2); // starting vertex
		for (int i = 0; i < xn - 1; i++)
		{
			triangles.push_back(a + i + 1); triangles.push_back(a + i); triangles.push_back(bottom);
		}
		triangles.push_back(a); triangles.push_back(a + xn - 1); triangles.push_back(bottom);

		return std::make_shared<Mesh>(std::move(vertices), std::move(triangles));
	}
	// generate internal triangles as return, and internal vertices will be added automatically
	std::vector<int> BigTriangleLerp(std::vector<Vertex>& vertices, const std::vector<int>& big_triangle, bool unit_vertex)
	{
		std::vector<int> triangles;
		size_t smooth = big_triangle.size() / 3;

		// add first part triangles
		triangles.push_back(big_triangle[0]); triangles.push_back(big_triangle[1]); triangles.push_back(big_triangle[3 * smooth - 1]);
		// add second part triangles
		for (size_t j = 1; j < smooth - 1; j++)
		{
			int a = big_triangle[j];                  // left upper vertex
			int b = big_triangle[j + 1];              // left below vertex
			int c = big_triangle[3 * smooth - j];     // right upper vertex
			int d = big_triangle[3 * smooth - j - 1]; // right below vertex
			Vertex vb = vertices[b], vd = vertices[d];
			for (size_t i = 0; i < j; i++)
			{
				// add vertex
				Vertex ve_new = VertexLerp(vb, vd, static_cast<float>(i + 1) / (j + 1));
				if (unit_vertex)
				{
					ve_new.p.w = 0;
					float length = VectorLength(ve_new.p);
					if (length != 0)
						ve_new.p /= length;
					ve_new.p.w = 1;
				}
				vertices.push_back(ve_new);
				int e = static_cast<int>(vertices.size()) - 1; // save new vertex in center below
				int f = e - static_cast<int>(j) + 1;         // load old vertex in center upper
				// add triangle
				triangles.push_back(a); triangles.push_back(b); triangles.push_back(e);
				if (i != j - 1)
				{
					triangles.push_back(a); triangles.push_back(e); triangles.push_back(f);
					a = f, b = e;
				}
				else
					b = e;
			}
			triangles.push_back(a); triangles.push_back(b); triangles.push_back(c);
			triangles.push_back(c); triangles.push_back(b); triangles.push_back(d);
		}
		// add last part triangles
		if (smooth > 1)
		{
			int a = big_triangle[smooth - 1];     // left upper vertex
			int b = big_triangle[smooth];         // left below vertex
			int c = big_triangle[2 * smooth + 1]; // right upper vertex
			int d = big_triangle[2 * smooth];     // right below vertex
			int f = static_cast<int>(vertices.size() - smooth) + 2; // load old vertex in center upper
			for (size_t i = 0; i < smooth - 1; i++)
			{
				int e = big_triangle[smooth + i + 1]; // center below vertex
				triangles.push_back(a); triangles.push_back(b); triangles.push_back(e);
				if (i != smooth - 2)
				{
					triangles.push_back(a); triangles.push_back(e); triangles.push_back(f);
					a = f++, b = e;
				}
				else
					b = e;
			}
			triangles.push_back(a); triangles.push_back(b); triangles.push_back(c);
			triangles.push_back(c); triangles.push_back(b); triangles.push_back(d);
		}

		return triangles;
	}
	void SphereSkeleton(std::vector<Vertex>& vertices, int smooth, int a, int b)
	{
		for (int i = 1; i < smooth; i++)
		{
			Vertex v_new = VertexLerp(vertices[a], vertices[b], static_cast<float>(i) / smooth);
			v_new.p.w = 0;
			float length = VectorLength(v_new.p);
			if (length != 0)
				v_new.p /= length;
			v_new.p.w = 1;
			vertices.push_back(v_new);
		}
	}
	std::vector<int> SphereBigTriangle(int smooth, int vn, int a, int b, int c, int eab, int ebc, int eca)
	{
		std::vector<int> big_tri;
		auto PushBackEdge = [&big_tri, smooth, vn](int e)
		{
			int o = vn + (abs(e) - 1) * (smooth - 1);
			if (e > 0)
			{
				for (int i = 0; i < smooth - 1; i++)
					big_tri.push_back(i + o);
			}
			else
			{
				for (int i = smooth - 2; i >= 0; i--)
					big_tri.push_back(i + o);
			}
		};
		big_tri.push_back(a);
		PushBackEdge(eab);
		big_tri.push_back(b);
		PushBackEdge(ebc);
		big_tri.push_back(c);
		PushBackEdge(eca);
		return big_tri;
	}
	void TrianglesAppend(std::vector<int>& triangles, const std::vector<int>& _triangles)
	{
		triangles.insert(triangles.end(), _triangles.begin(), _triangles.end());
	}
	std::shared_ptr<Mesh> CreateSphereMeshB(int smooth)
	{
		std::vector<Vertex> vertices;
		std::vector<int> triangles;

		// add first part vertices
		vertices.push_back(Point(0, 1, 0));
		vertices.push_back(Point(1, 0, 0));
		vertices.push_back(Point(0, 0, -1));
		vertices.push_back(Point(-1, 0, 0));
		vertices.push_back(Point(0, 0, 1));
		vertices.push_back(Point(0, -1, 0));
		// add second part vertices
		SphereSkeleton(vertices, smooth, 0, 1); SphereSkeleton(vertices, smooth, 0, 2);
		SphereSkeleton(vertices, smooth, 0, 3); SphereSkeleton(vertices, smooth, 0, 4);
		SphereSkeleton(vertices, smooth, 1, 2); SphereSkeleton(vertices, smooth, 2, 3);
		SphereSkeleton(vertices, smooth, 3, 4); SphereSkeleton(vertices, smooth, 4, 1);
		SphereSkeleton(vertices, smooth, 1, 5); SphereSkeleton(vertices, smooth, 2, 5);
		SphereSkeleton(vertices, smooth, 3, 5); SphereSkeleton(vertices, smooth, 4, 5);
		// update normal
		for (auto& v : vertices)
		{
			v.n = v.p;
			v.n.w = 0;
		}
		// add triangles
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 0, 1, 2, +1, +5, -2), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 0, 2, 3, +2, +6, -3), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 0, 3, 4, +3, +7, -4), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 0, 4, 1, +4, +8, -1), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 5, 2, 1, -10, -5, +9), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 5, 3, 2, -11, -6, +10), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 5, 4, 3, -12, -7, +11), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 5, 1, 4, -9, -8, +12), true));

		return std::make_shared<Mesh>(std::move(vertices), std::move(triangles));
	}
	std::shared_ptr<Mesh> CreateSphereMeshC(int smooth)
	{
		std::vector<Vertex> vertices;
		std::vector<int> triangles;

		// add first part vertices
		float sqr2 = sqrtf(2), sqr6 = sqrtf(6);
		vertices.push_back(Point(0, 1, 0));
		vertices.push_back(Point(2 * sqr2 / 3, -1.0f / 3, 0));
		vertices.push_back(Point(-sqr2 / 3, -1.0f / 3, -sqr6 / 3));
		vertices.push_back(Point(-sqr2 / 3, -1.0f / 3, +sqr6 / 3));
		// add second part vertices
		SphereSkeleton(vertices, smooth, 0, 1); SphereSkeleton(vertices, smooth, 0, 2);
		SphereSkeleton(vertices, smooth, 0, 3); SphereSkeleton(vertices, smooth, 1, 2);
		SphereSkeleton(vertices, smooth, 2, 3); SphereSkeleton(vertices, smooth, 3, 1);
		// update normal
		for (auto& v : vertices)
		{
			v.n = v.p;
			v.n.w = 0;
		}
		// add triangles
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 4, 0, 1, 2, +1, +4, -2), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 4, 0, 2, 3, +2, +5, -3), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 4, 0, 3, 1, +3, +6, -1), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 4, 1, 3, 2, -6, -5, -4), true));

		return std::make_shared<Mesh>(std::move(vertices), std::move(triangles));
	}
	std::shared_ptr<Mesh> CreateSphereMeshD(int smooth)
	{
		std::vector<Vertex> vertices;
		std::vector<int> triangles;

		// add first part vertices
		float sqr3 = sqrtf(3), sqr5 = sqrtf(5);
		vertices.push_back(Point(1, sqr3, (3 + sqr5) / 2));
		vertices.push_back(Point(-2, 0, (3 + sqr5) / 2));
		vertices.push_back(Point(1, -sqr3, (3 + sqr5) / 2));
		vertices.push_back(Point(-(1 + sqr5) / 2, -(1 + sqr5) * sqr3 / 2, (sqr5 - 1) / 2));
		vertices.push_back(Point(1 + sqr5, 0, (sqr5 - 1) / 2));
		vertices.push_back(Point(-(1 + sqr5) / 2, (1 + sqr5) * sqr3 / 2, (sqr5 - 1) / 2));
		vertices.push_back(Point((1 + sqr5) / 2, (1 + sqr5) * sqr3 / 2, -(sqr5 - 1) / 2));
		vertices.push_back(Point(-(1 + sqr5), 0, -(sqr5 - 1) / 2));
		vertices.push_back(Point((1 + sqr5) / 2, -(1 + sqr5) * sqr3 / 2, -(sqr5 - 1) / 2));
		vertices.push_back(Point(-1, -sqr3, -(3 + sqr5) / 2));
		vertices.push_back(Point(2, 0, -(3 + sqr5) / 2));
		vertices.push_back(Point(-1, sqr3, -(3 + sqr5) / 2));
		for (auto& v : vertices)
		{
			float length = VectorLength(v.p);
			if (length != 0)
				v.p /= length;
			v.p.w = 1;
		}
		// add second part vertices
		SphereSkeleton(vertices, smooth, 0, 1); SphereSkeleton(vertices, smooth, 0, 2);
		SphereSkeleton(vertices, smooth, 0, 4); SphereSkeleton(vertices, smooth, 0, 6);
		SphereSkeleton(vertices, smooth, 0, 5); SphereSkeleton(vertices, smooth, 1, 2);
		SphereSkeleton(vertices, smooth, 2, 4); SphereSkeleton(vertices, smooth, 4, 6);
		SphereSkeleton(vertices, smooth, 6, 5); SphereSkeleton(vertices, smooth, 5, 1);
		// 1 2 4 6 5
		SphereSkeleton(vertices, smooth, 1, 7); SphereSkeleton(vertices, smooth, 1, 3);
		SphereSkeleton(vertices, smooth, 2, 3); SphereSkeleton(vertices, smooth, 2, 8);
		SphereSkeleton(vertices, smooth, 4, 8); SphereSkeleton(vertices, smooth, 4, 10);
		SphereSkeleton(vertices, smooth, 6, 10); SphereSkeleton(vertices, smooth, 6, 11);
		SphereSkeleton(vertices, smooth, 5, 11); SphereSkeleton(vertices, smooth, 5, 7);
		// 7 3 8 10 11
		SphereSkeleton(vertices, smooth, 7, 3); SphereSkeleton(vertices, smooth, 3, 8);
		SphereSkeleton(vertices, smooth, 8, 10); SphereSkeleton(vertices, smooth, 10, 11);
		SphereSkeleton(vertices, smooth, 11, 7); SphereSkeleton(vertices, smooth, 7, 9);
		SphereSkeleton(vertices, smooth, 3, 9); SphereSkeleton(vertices, smooth, 8, 9);
		SphereSkeleton(vertices, smooth, 10, 9); SphereSkeleton(vertices, smooth, 11, 9);
		// update normal
		for (auto& v : vertices)
		{
			v.n = v.p;
			v.n.w = 0;
		}
		// add triangles
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 0, 1, 2, +1, +6, -2), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 0, 2, 4, +2, +7, -3), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 0, 4, 6, +3, +8, -4), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 0, 6, 5, +4, +9, -5), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 0, 5, 1, +5, +10, -1), true));
		// 1 2 4 6 5
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 1, 7, 3, +11, +21, -12), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 1, 3, 2, +12, -13, -6), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 2, 3, 8, +13, +22, -14), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 2, 8, 4, +14, -15, -7), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 4, 8, 10, +15, +23, -16), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 4, 10, 6, +16, -17, -8), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 6, 10, 11, +17, +24, -18), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 6, 11, 5, +18, -19, -9), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 5, 11, 7, +19, +25, -20), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 5, 7, 1, +20, -11, -10), true));
		// 7 3 8 10 11
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 9, 3, 7, -27, -21, +26), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 9, 8, 3, -28, -22, +27), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 9, 10, 8, -29, -23, +28), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 9, 11, 10, -30, -24, +29), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 12, 9, 7, 11, -26, -25, +30), true));

		return std::make_shared<Mesh>(std::move(vertices), std::move(triangles));
	}
	std::shared_ptr<Mesh> CreateMeshFromObjFile(const std::string& filename)
	{
		std::vector<Vertex> vertices;
		std::vector<int> triangles;

		std::ifstream fs(filename.c_str());
		std::vector<Point> vs;
		std::vector<UV> vts;
		std::vector<Color> vns;

		while (fs.good())
		{
			std::string line;
			std::getline(fs, line);
			std::istringstream ss(line);

			std::string str;
			ss >> str;
			if (str.length() == 0) // empty line
				continue;
			else if (str[0] == '#') // comment line
				continue;
			else if (str == "mtlib") // material // ignore
				continue;
			else if (str == "usemtl") // material // ignore
				continue;
			else if (str == "g") // group // ignore
				continue;
			else if (str == "v") // pos
			{
				Point p;
				ss >> p.x >> p.y >> p.z;
				vs.push_back(p);
			}
			else if (str == "vt") // uv
			{
				UV uv;
				ss >> uv.x >> uv.y;
				vts.push_back(uv);
			}
			else if (str == "vn") // normal
			{
				Color c;
				ss >> c.x >> c.y >> c.z;
				vns.push_back(c);
			}
			else if (str == "f") // face
			{
				for (int i = 0; i < 3; i++)
				{
					int values[3]{ 0,0,0 };

					std::string part;
					ss >> part;
					std::istringstream part_ss(part);
					for (int j = 0; j < 3; j++)
					{
						std::string value_str;
						getline(part_ss, value_str, '/');
						if (value_str.length() != 0)
							values[j] = std::stoi(value_str);
					}
					for (int j = 0; j < 3; j++)
						values[j] -= 1;

					triangles.push_back(static_cast<int>(vertices.size()));
					if (values[0] < 0 || values[0] >= vs.size()) // no pos, ignore the face
						break;
					Vertex v(vs[values[0]]);
					if (values[2] >= 0 && values[2] < vns.size())
						v.c = vns[values[2]];
					if (values[1] >= 0 && values[1] < vts.size())
						v.uv = vts[values[1]];
					vertices.push_back(v);
				}
			}
			else // other // ignore
				continue;
		}

		return std::make_shared<Mesh>(std::move(vertices), std::move(triangles));
	}
	Texture::Texture(int _width, int _height) : width(_width), height(_height), buffer(new Color[static_cast<size_t>(_width) * _height])
	{
	}
	Texture::Texture(const Texture& t) : width(t.width), height(t.height), buffer(new Color[static_cast<size_t>(t.width) * t.height])
	{
		std::copy(t.buffer, t.buffer + static_cast<size_t>(width) * height, buffer);
	}
	Texture::~Texture()
	{
		delete[] buffer;
	}
	Color Texture::GetColor(float u, float v)
	{
		int x = static_cast<int>(u * width);
		int y = static_cast<int>(v * height);
		x = Clamp(x, 0, width - 1);
		y = Clamp(y, 0, height - 1);
		return buffer[y * width + x];
	}
	Color Texture::GetColor(UV uv)
	{
		return GetColor(uv.x, uv.y);
	}
	std::shared_ptr<Texture> CreateTexture1()
	{
		char image1[49]{
			0,0,0,0,0,0,0,
			0,0,1,0,0,0,0,
			0,1,1,0,0,0,0,
			0,0,1,0,0,0,0,
			0,0,1,0,0,0,0,
			0,1,1,1,0,0,0,
			0,0,0,0,0,0,0
		};
		auto pt = std::make_shared<Texture>(7, 7);
		for (int i = 0; i < 49; i++)
			pt->buffer[i] = (image1[i] + 1) / 2.0f * Color(1, 1, 1);
		return pt;
	}
	std::shared_ptr<Texture> CreateTextureC()
	{
		char imageC[49]{
			0,0,0,0,0,0,0,
			0,0,1,1,0,0,0,
			0,1,0,0,1,0,0,
			0,1,0,0,0,0,0,
			0,1,0,0,1,0,0,
			0,0,1,1,0,0,0,
			0,0,0,0,0,0,0
		};
		auto pt = std::make_shared<Texture>(7, 7);
		for (int i = 0; i < 49; i++)
			pt->buffer[i] = (imageC[i] + 1) / 2.0f * Color(1, 1, 1);
		return pt;
	}
	std::shared_ptr<Texture> CreateTexturePlaid()
	{
		auto pt = std::make_shared<Texture>(9, 9);
		for (int i = 0; i < 81; i++)
		{
			if (i % 2 == 0)
				pt->buffer[i] = Color(1, 1, 1);
			else
				pt->buffer[i] = Color(0, 0, 1);
		}
		return pt;
	}
	std::shared_ptr<Texture> CreateTextureDice()
	{
		const int size = 28;
		char image[size * size]{
			0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,0,0,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,0,0,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,

			0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,1,0,1,0,0, 0,0,0,0,1,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
			0,0,1,0,1,0,0, 0,0,0,0,1,0,0, 0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,1,1,1,0,0, 0,0,0,0,1,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,1,0,0, 0,0,0,0,1,0,0, 0,0,0,0,1,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,1,0,0, 0,0,0,0,1,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,

			0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,0,0,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,

			0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,0,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,1,1,1,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
			0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0,0
		};
		auto pt = std::make_shared<Texture>(size, size);
		for (int i = 0; i < size * size; i++)
			pt->buffer[i] = (image[i] + 1) / 2.0f * Color(1, 1, 1);
		return pt;
	}
}