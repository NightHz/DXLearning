#pragma once
#include "math.h"
#include "mesh.h"

namespace Rehenz
{
	bool ClipPointInside(Point2 p);
	bool ClipPointInside(Point3 p);
	bool ClipPointInside(Point p);

	bool ClipLine2DCohenSutherland(Point2& p1, Point2& p2, float _xmax, float _ymax);
	bool ClipLine2DCohenSutherland(Point2& p1, Point2& p2, float _xmin, float _xmax, float _ymin, float _ymax);

	bool ClipLine3DCohenSutherland(Point3& p1, Point3& p2, float _xmax, float _ymax, float _zmax);
	bool ClipLine3DCohenSutherland(Point3& p1, Point3& p2, float _xmin, float _xmax, float _ymin, float _ymax, float _zmin, float _zmax);

	bool ClipLineCohenSutherland(Point& p1, Point& p2);

	void ClipTriangleCohenSutherland(std::vector<Vertex>& vertices, std::vector<int>& triangles, int _a, int _b, int _c);
}