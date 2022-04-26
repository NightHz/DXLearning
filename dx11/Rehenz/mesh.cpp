#include "mesh.h"

namespace Rehenz
{
	Vertex::Vertex(Point _p) : p(_p), c(1, 1, 1), uv(0, 0), uv2(0, 0)
	{
		coef = 1;
	}

	Vertex::Vertex(Point _p, Color _c, UV _uv, UV _uv2, float _rhw)
		: p(_p), c(_c), uv(_uv), uv2(_uv2)
	{
		coef = _rhw;
	}

	Vertex::Vertex(Point _p, UV _uv, UV _uv2)
		: p(_p), c(1, 1, 1), uv(_uv), uv2(_uv2)
	{
		coef = 1;
	}

	Vertex& Vertex::operator*=(float f)
	{
		p *= f;
		c *= f;
		uv *= f;
		uv2 *= f;
		coef *= f;
		return *this;
	}

	Vertex& Vertex::operator+=(const Vertex& v)
	{
		p += v.p;
		c += v.c;
		uv += v.uv;
		uv2 += v.uv2;
		coef += v.coef;
		return *this;
	}

	Vertex Vertex::operator*(float f) const
	{
		return Vertex(p * f, c * f, uv * f, uv2 * f, coef * f);
	}

	Vertex Vertex::operator+(const Vertex& v) const
	{
		return Vertex(p + v.p, c + v.c, uv + v.uv, uv2 + v.uv2, coef + v.coef);
	}

	Vertex Vertex::operator-(const Vertex& v) const
	{
		return Vertex(p - v.p, c - v.c, uv - v.uv, uv2 - v.uv2, coef - v.coef);
	}

	void VertexNormalize(Vertex& v)
	{
		v *= 1 / v.coef;
	}

	Vertex VertexLerp(const Vertex& v1, const Vertex& v2, float t)
	{
		return Vertex(PointLerp(v1.p, v2.p, t), Lerp(v1.c, v2.c, t), Lerp(v1.uv, v2.uv, t), Lerp(v1.uv2, v2.uv2, t), Lerp(v1.coef, v2.coef, t));
	}



	Mesh::Mesh() : vertices(), triangles()
	{
	}
	Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<int>& _triangles) : vertices(_vertices), triangles(_triangles)
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
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, -0.5f), GetColor(0), UV(0, 1), UV(0.25f, 0.5f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, -0.5f), GetColor(1), UV(1, 1), UV(0.5f, 0.5f)));
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, -0.5f), GetColor(2), UV(0, 0), UV(0.25f, 0.25f)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, -0.5f), GetColor(3), UV(1, 0), UV(0.5f, 0.25f)));
		triangles.push_back(0); triangles.push_back(2); triangles.push_back(1);
		triangles.push_back(2); triangles.push_back(3); triangles.push_back(1);
		// down
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, 0.5f), GetColor(4), UV(0, 1), UV(0.25f, 0.75f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, 0.5f), GetColor(5), UV(1, 1), UV(0.5f, 0.75f)));
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, -0.5f), GetColor(0), UV(0, 0), UV(0.25f, 0.5f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, -0.5f), GetColor(1), UV(1, 0), UV(0.5f, 0.5f)));
		triangles.push_back(4); triangles.push_back(6); triangles.push_back(5);
		triangles.push_back(6); triangles.push_back(7); triangles.push_back(5);
		// up
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, -0.5f), GetColor(2), UV(0, 1), UV(0.25f, 0.25f)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, -0.5f), GetColor(3), UV(1, 1), UV(0.5f, 0.25f)));
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, 0.5f), GetColor(6), UV(0, 0), UV(0.25f, 0)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, 0.5f), GetColor(7), UV(1, 0), UV(0.5f, 0)));
		triangles.push_back(8); triangles.push_back(10); triangles.push_back(9);
		triangles.push_back(10); triangles.push_back(11); triangles.push_back(9);
		// back
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, 0.5f), GetColor(6), UV(0, 1), UV(0.25f, 1)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, 0.5f), GetColor(7), UV(1, 1), UV(0.5f, 1)));
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, 0.5f), GetColor(4), UV(0, 0), UV(0.25f, 0.75f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, 0.5f), GetColor(5), UV(1, 0), UV(0.5f, 0.75f)));
		triangles.push_back(12); triangles.push_back(14); triangles.push_back(13);
		triangles.push_back(14); triangles.push_back(15); triangles.push_back(13);
		// left
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, 0.5f), GetColor(4), UV(0, 1), UV(0, 0.5f)));
		vertices.push_back(Vertex(Point(-0.5f, -0.5f, -0.5f), GetColor(0), UV(1, 1), UV(0.25f, 0.5f)));
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, 0.5f), GetColor(6), UV(0, 0), UV(0, 0.25f)));
		vertices.push_back(Vertex(Point(-0.5f, 0.5f, -0.5f), GetColor(2), UV(1, 0), UV(0.25f, 0.25f)));
		triangles.push_back(16); triangles.push_back(18); triangles.push_back(17);
		triangles.push_back(18); triangles.push_back(19); triangles.push_back(17);
		// right
		vertices.push_back(Vertex(Point(0.5f, -0.5f, -0.5f), GetColor(1), UV(0, 1), UV(0.5f, 0.5f)));
		vertices.push_back(Vertex(Point(0.5f, -0.5f, 0.5f), GetColor(5), UV(1, 1), UV(0.75f, 0.5f)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, -0.5f), GetColor(3), UV(0, 0), UV(0.5f, 0.25f)));
		vertices.push_back(Vertex(Point(0.5f, 0.5f, 0.5f), GetColor(7), UV(1, 0), UV(0.75f, 0.25f)));
		triangles.push_back(20); triangles.push_back(22); triangles.push_back(21);
		triangles.push_back(22); triangles.push_back(23); triangles.push_back(21);

		return std::make_shared<Mesh>(vertices, triangles);
	}
	std::shared_ptr<Mesh> CreateSphereMesh(int smooth)
	{
		smooth = Clamp(smooth, 2, 200);
		int yn = smooth, xn = 2 * smooth;

		std::vector<Vertex> vertices;
		std::vector<int> triangles;

		// add vertices by latitude from top(0,1,0) to bottom(0,-1,0)
		vertices.push_back(Point(0, 1, 0));
		for (int y = 1; y < yn; y++)
		{
			float theta = pi * y / yn;
			for (int x = 0; x < xn; x++)
			{
				float phi = 2 * pi * x / xn;
				vertices.push_back(Point(sinf(theta) * cosf(phi), cosf(theta), -sinf(theta) * sinf(phi)));
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

		return std::make_shared<Mesh>(vertices, triangles);
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
		// add triangles
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 0, 1, 2, +1, +5, -2), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 0, 2, 3, +2, +6, -3), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 0, 3, 4, +3, +7, -4), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 0, 4, 1, +4, +8, -1), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 5, 2, 1, -10, -5, +9), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 5, 3, 2, -11, -6, +10), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 5, 4, 3, -12, -7, +11), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 6, 5, 1, 4, -9, -8, +12), true));

		return std::make_shared<Mesh>(vertices, triangles);
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
		// add triangles
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 4, 0, 1, 2, +1, +4, -2), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 4, 0, 2, 3, +2, +5, -3), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 4, 0, 3, 1, +3, +6, -1), true));
		TrianglesAppend(triangles, BigTriangleLerp(vertices, SphereBigTriangle(smooth, 4, 1, 3, 2, -6, -5, -4), true));

		return std::make_shared<Mesh>(vertices, triangles);
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

		return std::make_shared<Mesh>(vertices, triangles);
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