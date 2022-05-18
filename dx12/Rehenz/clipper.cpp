#include "clipper.h"
#include <stack>

namespace Rehenz
{
	namespace ClipState
	{
		const int inside = 0; // 000000
		const int left = 1;   // 000001
		const int right = 2;  // 000010
		const int down = 4;   // 000100
		const int top = 8;    // 001000
		const int back = 16;  // 010000
		const int front = 32; // 100000
	};

	float xmin = 0, xmax = 1;
	float ymin = 0, ymax = 1;
	float zmin = 0, zmax = 1;

	int ComputeClipState(Point2 p)
	{
		int state = ClipState::inside;
		if (p.x < xmin)      state |= ClipState::left;
		else if (p.x > xmax) state |= ClipState::right;
		if (p.y < ymin)      state |= ClipState::down;
		else if (p.y > ymax) state |= ClipState::top;
		return state;
	}

	int ComputeClipState(Point3 p)
	{
		int state = ClipState::inside;
		if (p.x < xmin)      state |= ClipState::left;
		else if (p.x > xmax) state |= ClipState::right;
		if (p.y < ymin)      state |= ClipState::down;
		else if (p.y > ymax) state |= ClipState::top;
		if (p.z < zmin)      state |= ClipState::back;
		else if (p.z > zmax) state |= ClipState::front;
		return state;
	}

	int ComputeClipState(Point p)
	{
		int state = ClipState::inside;
		if (p.x < -p.w)     state |= ClipState::left;
		else if (p.x > p.w) state |= ClipState::right;
		if (p.y < -p.w)     state |= ClipState::down;
		else if (p.y > p.w) state |= ClipState::top;
		if (p.z < 0)        state |= ClipState::back;
		else if (p.z > p.w) state |= ClipState::front;
		return state;
	}

	bool ClipPointInside(Point2 p)
	{
		return ComputeClipState(p) == ClipState::inside;
	}

	bool ClipPointInside(Point3 p)
	{
		return ComputeClipState(p) == ClipState::inside;
	}

	bool ClipPointInside(Point p)
	{
		return ComputeClipState(p) == ClipState::inside;
	}

	bool ClipLine2DCohenSutherland(Point2& p1, Point2& p2, float _xmax, float _ymax)
	{
		return ClipLine2DCohenSutherland(p1, p2, 0, _xmax, 0, _ymax);
	}

	bool ClipLine2DCohenSutherland(Point2& p1, Point2& p2, float _xmin, float _xmax, float _ymin, float _ymax)
	{
		// paste from https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm

		xmin = _xmin, xmax = _xmax;
		ymin = _ymin, ymax = _ymax;
		int p1_state = ComputeClipState(p1);
		int p2_state = ComputeClipState(p2);

		while (true)
		{
			if (p1_state == ClipState::inside && p2_state == ClipState::inside)
			{
				// bitwise OR is 0: both points inside window; trivially accept and exit loop
				return true;
			}
			else if (p1_state & p2_state)
			{
				// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
				// or BOTTOM), so both must be outside window; exit loop (accept is false)
				return false;
			}
			else
			{
				// failed both tests, so calculate the line segment to clip
				// from an outside point to an intersection with clip edge
				float x = 0, y = 0;

				// At least one endpoint is outside the clip rectangle; pick it.
				int outcodeOut = p2_state > p1_state ? p2_state : p1_state;

				// Now find the intersection point;
				// use formulas:
				//   slope = (y1 - y0) / (x1 - x0)
				//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
				//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
				// No need to worry about divide-by-zero because, in each case, the
				// outcode bit being tested guarantees the denominator is non-zero
				if (outcodeOut & ClipState::top)          // point is above the clip window
				{
					x = p1.x + (p2.x - p1.x) * (ymax - p1.y) / (p2.y - p1.y);
					y = ymax;
				}
				else if (outcodeOut & ClipState::down)    // point is below the clip window
				{
					x = p1.x + (p2.x - p1.x) * (ymin - p1.y) / (p2.y - p1.y);
					y = ymin;
				}
				else if (outcodeOut & ClipState::right)   // point is to the right of clip window
				{
					y = p1.y + (p2.y - p1.y) * (xmax - p1.x) / (p2.x - p1.x);
					x = xmax;
				}
				else if (outcodeOut & ClipState::left)    // point is to the left of clip window
				{
					y = p1.y + (p2.y - p1.y) * (xmin - p1.x) / (p2.x - p1.x);
					x = xmin;
				}

				// Now we move outside point to intersection point to clip
				// and get ready for next pass.
				if (outcodeOut == p1_state)
				{
					p1 = Point2(x, y);
					p1_state = ComputeClipState(p1);
				}
				else
				{
					p2 = Point2(x, y);
					p2_state = ComputeClipState(p2);
				}
			}
		}
	}

	bool ClipLine3DCohenSutherland(Point3& p1, Point3& p2, float _xmax, float _ymax, float _zmax)
	{
		return ClipLine3DCohenSutherland(p1, p2, 0, _xmax, 0, _ymax, 0, _zmax);
	}

	bool ClipLine3DCohenSutherland(Point3& p1, Point3& p2, float _xmin, float _xmax, float _ymin, float _ymax, float _zmin, float _zmax)
	{
		xmin = _xmin, xmax = _xmax;
		ymin = _ymin, ymax = _ymax;
		zmin = _zmin, zmax = _zmax;
		int p1_state = ComputeClipState(p1);
		int p2_state = ComputeClipState(p2);

		while (true)
		{
			if (p1_state == ClipState::inside && p2_state == ClipState::inside)
			{
				return true;
			}
			else if (p1_state & p2_state)
			{
				return false;
			}
			else
			{
				float x = 0, y = 0, z = 0;
				Point3 dp = p2 - p1;
				int state = p2_state > p1_state ? p2_state : p1_state;
				if (state & ClipState::front)
				{
					x = p1.x + dp.x * (zmax - p1.z) / dp.z;
					y = p1.y + dp.y * (zmax - p1.z) / dp.z;
					z = zmax;
				}
				else if (state & ClipState::back)
				{
					x = p1.x + dp.x * (zmin - p1.z) / dp.z;
					y = p1.y + dp.y * (zmin - p1.z) / dp.z;
					z = zmin;
				}
				else if (state & ClipState::top)
				{
					x = p1.x + dp.x * (ymax - p1.y) / dp.y;
					y = ymax;
					z = p1.z + dp.z * (ymax - p1.y) / dp.y;
				}
				else if (state & ClipState::down)
				{
					x = p1.x + dp.x * (ymin - p1.y) / dp.y;
					y = ymin;
					z = p1.z + dp.z * (ymin - p1.y) / dp.y;
				}
				else if (state & ClipState::right)
				{
					x = xmax;
					y = p1.y + dp.y * (xmax - p1.x) / dp.x;
					z = p1.z + dp.z * (xmax - p1.x) / dp.x;
				}
				else if (state & ClipState::left)
				{
					x = xmin;
					y = p1.y + dp.y * (xmin - p1.x) / dp.x;
					z = p1.z + dp.z * (xmin - p1.x) / dp.x;
				}

				if (state == p1_state)
				{
					p1 = Point3(x, y, z);
					p1_state = ComputeClipState(p1);
				}
				else
				{
					p2 = Point3(x, y, z);
					p2_state = ComputeClipState(p2);
				}
			}
		}
	}

	bool ClipLineCohenSutherland(Point& p1, Point& p2)
	{
		int p1_state = ComputeClipState(p1);
		int p2_state = ComputeClipState(p2);

		while (true)
		{
			if (p1_state == ClipState::inside && p2_state == ClipState::inside)
			{
				return true;
			}
			else if (p1_state & p2_state)
			{
				return false;
			}
			else
			{
				Point p;
				Point dp = p2 - p1;
				int state = p2_state > p1_state ? p2_state : p1_state;
				if (state & ClipState::front)
				{
					p = p1 + dp * ((p1.w - p1.z) / (dp.z - dp.w));
					p.z = p.w;
				}
				else if (state & ClipState::back)
				{
					p = p1 + dp * (-p1.z / dp.z);
					p.z = 0;
				}
				else if (state & ClipState::top)
				{
					p = p1 + dp * ((p1.w - p1.y) / (dp.y - dp.w));
					p.y = p.w;
				}
				else if (state & ClipState::down)
				{
					p = p1 + dp * ((-p1.w - p1.y) / (dp.y + dp.w));
					p.y = -p.w;
				}
				else if (state & ClipState::right)
				{
					p = p1 + dp * ((p1.w - p1.x) / (dp.x - dp.w));
					p.x = p.w;
				}
				else if (state & ClipState::left)
				{
					p = p1 + dp * ((-p1.w - p1.x) / (dp.x + dp.w));
					p.x = -p.w;
				}

				if (state == p1_state)
				{
					p1 = p;
					p1_state = ComputeClipState(p1);
				}
				else
				{
					p2 = p;
					p2_state = ComputeClipState(p2);
				}
			}
		}
	}

	void ClipTriangleCohenSutherland(std::vector<Vertex>& vertices, std::vector<int>& triangles, int _a, int _b, int _c)
	{
		std::stack<int> tris_wait_clip;
		tris_wait_clip.push(_c); tris_wait_clip.push(_b); tris_wait_clip.push(_a);
		while (!tris_wait_clip.empty())
		{
			// get three vertices of the triangle
			int a = tris_wait_clip.top(); tris_wait_clip.pop();
			int b = tris_wait_clip.top(); tris_wait_clip.pop();
			int c = tris_wait_clip.top(); tris_wait_clip.pop();

			// Cohen Sutherland algorithm
			// compute clip state
			int a_state = ComputeClipState(vertices[a].p);
			int b_state = ComputeClipState(vertices[b].p);
			int c_state = ComputeClipState(vertices[c].p);

			while (true)
			{
				// 1 if all inside
				if (a_state == ClipState::inside && b_state == ClipState::inside && c_state == ClipState::inside)
				{
					triangles.push_back(a); triangles.push_back(b); triangles.push_back(c);
					break;
				}
				// 2 if all in one side
				else if (a_state & b_state & c_state)
				{
					break;
				}
				// 3 if need clip
				else
				{
					// sort to a >= b,c
					if (b_state > a_state && b_state >= c_state)
					{
						int temp_state = a_state;
						int temp = a;
						a_state = b_state, a = b;
						b_state = c_state, b = c;
						c_state = temp_state, c = temp;
					}
					else if (c_state > a_state && c_state >= b_state)
					{
						int temp_state = a_state;
						int temp = a;
						a_state = c_state, a = c;
						c_state = b_state, c = b;
						b_state = temp_state, b = temp;
					}
					Vertex& va = vertices[a], & vb = vertices[b], & vc = vertices[c];
					Vertex dv(Point(0)), vd(Point(0)), ve(Point(0));
					int condition = 0;

					// compute clip point
					if (a_state & ClipState::front)
					{
						// 3-x-1 if a,b outside
						if (b_state & ClipState::front)
						{
							condition = 1;
							dv = va - vc;
							vd = vc + dv * ((vc.p.w - vc.p.z) / (dv.p.z - dv.p.w));
							vd.p.z = vd.p.w;
							dv = vb - vc;
							ve = vc + dv * ((vc.p.w - vc.p.z) / (dv.p.z - dv.p.w));
							ve.p.z = ve.p.w;
						}
						// 3-x-2 if a,c outside
						else if (c_state & ClipState::front)
						{
							condition = 2;
							dv = va - vb;
							vd = vb + dv * ((vb.p.w - vb.p.z) / (dv.p.z - dv.p.w));
							vd.p.z = vd.p.w;
							dv = vc - vb;
							ve = vb + dv * ((vb.p.w - vb.p.z) / (dv.p.z - dv.p.w));
							ve.p.z = ve.p.w;
						}
						// 3-x-3 if only a outside
						else
						{
							condition = 3;
							dv = va - vb;
							vd = vb + dv * ((vb.p.w - vb.p.z) / (dv.p.z - dv.p.w));
							vd.p.z = vd.p.w;
							dv = va - vc;
							ve = vc + dv * ((vc.p.w - vc.p.z) / (dv.p.z - dv.p.w));
							ve.p.z = ve.p.w;
						}
					}
					else if (a_state & ClipState::back)
					{
						if (b_state & ClipState::back)
						{
							condition = 1;
							dv = va - vc;
							vd = vc + dv * (-vc.p.z / dv.p.z);
							vd.p.z = 0;
							dv = vb - vc;
							ve = vc + dv * (-vc.p.z / dv.p.z);
							ve.p.z = 0;
						}
						else if (c_state & ClipState::back)
						{
							condition = 2;
							dv = va - vb;
							vd = vb + dv * (-vb.p.z / dv.p.z);
							vd.p.z = 0;
							dv = vc - vb;
							ve = vb + dv * (-vb.p.z / dv.p.z);
							ve.p.z = 0;
						}
						else
						{
							condition = 3;
							dv = va - vb;
							vd = vb + dv * (-vb.p.z / dv.p.z);
							vd.p.z = 0;
							dv = va - vc;
							ve = vc + dv * (-vc.p.z / dv.p.z);
							ve.p.z = 0;
						}
					}
					else if (a_state & ClipState::top)
					{
						if (b_state & ClipState::top)
						{
							condition = 1;
							dv = va - vc;
							vd = vc + dv * ((vc.p.w - vc.p.y) / (dv.p.y - dv.p.w));
							vd.p.y = vd.p.w;
							dv = vb - vc;
							ve = vc + dv * ((vc.p.w - vc.p.y) / (dv.p.y - dv.p.w));
							ve.p.y = ve.p.w;
						}
						else if (c_state & ClipState::top)
						{
							condition = 2;
							dv = va - vb;
							vd = vb + dv * ((vb.p.w - vb.p.y) / (dv.p.y - dv.p.w));
							vd.p.y = vd.p.w;
							dv = vc - vb;
							ve = vb + dv * ((vb.p.w - vb.p.y) / (dv.p.y - dv.p.w));
							ve.p.y = ve.p.w;
						}
						else
						{
							condition = 3;
							dv = va - vb;
							vd = vb + dv * ((vb.p.w - vb.p.y) / (dv.p.y - dv.p.w));
							vd.p.y = vd.p.w;
							dv = va - vc;
							ve = vc + dv * ((vc.p.w - vc.p.y) / (dv.p.y - dv.p.w));
							ve.p.y = ve.p.w;
						}
					}
					else if (a_state & ClipState::down)
					{
						if (b_state & ClipState::down)
						{
							condition = 1;
							dv = va - vc;
							vd = vc + dv * ((-vc.p.w - vc.p.y) / (dv.p.y + dv.p.w));
							vd.p.y = -vd.p.w;
							dv = vb - vc;
							ve = vc + dv * ((-vc.p.w - vc.p.y) / (dv.p.y + dv.p.w));
							ve.p.y = -ve.p.w;
						}
						else if (c_state & ClipState::down)
						{
							condition = 2;
							dv = va - vb;
							vd = vb + dv * ((-vb.p.w - vb.p.y) / (dv.p.y + dv.p.w));
							vd.p.y = -vd.p.w;
							dv = vc - vb;
							ve = vb + dv * ((-vb.p.w - vb.p.y) / (dv.p.y + dv.p.w));
							ve.p.y = -ve.p.w;
						}
						else
						{
							condition = 3;
							dv = va - vb;
							vd = vb + dv * ((-vb.p.w - vb.p.y) / (dv.p.y + dv.p.w));
							vd.p.y = -vd.p.w;
							dv = va - vc;
							ve = vc + dv * ((-vc.p.w - vc.p.y) / (dv.p.y + dv.p.w));
							ve.p.y = -ve.p.w;
						}
					}
					else if (a_state & ClipState::right)
					{
						if (b_state & ClipState::right)
						{
							condition = 1;
							dv = va - vc;
							vd = vc + dv * ((vc.p.w - vc.p.x) / (dv.p.x - dv.p.w));
							vd.p.x = vd.p.w;
							dv = vb - vc;
							ve = vc + dv * ((vc.p.w - vc.p.x) / (dv.p.x - dv.p.w));
							ve.p.x = ve.p.w;
						}
						else if (c_state & ClipState::right)
						{
							condition = 2;
							dv = va - vb;
							vd = vb + dv * ((vb.p.w - vb.p.x) / (dv.p.x - dv.p.w));
							vd.p.x = vd.p.w;
							dv = vc - vb;
							ve = vb + dv * ((vb.p.w - vb.p.x) / (dv.p.x - dv.p.w));
							ve.p.x = ve.p.w;
						}
						else
						{
							condition = 3;
							dv = va - vb;
							vd = vb + dv * ((vb.p.w - vb.p.x) / (dv.p.x - dv.p.w));
							vd.p.x = vd.p.w;
							dv = va - vc;
							ve = vc + dv * ((vc.p.w - vc.p.x) / (dv.p.x - dv.p.w));
							ve.p.x = ve.p.w;
						}
					}
					else if (a_state & ClipState::left)
					{
						if (b_state & ClipState::left)
						{
							condition = 1;
							dv = va - vc;
							vd = vc + dv * ((-vc.p.w - vc.p.x) / (dv.p.x + dv.p.w));
							vd.p.x = -vd.p.w;
							dv = vb - vc;
							ve = vc + dv * ((-vc.p.w - vc.p.x) / (dv.p.x + dv.p.w));
							ve.p.x = -ve.p.w;
						}
						else if (c_state & ClipState::left)
						{
							condition = 2;
							dv = va - vb;
							vd = vb + dv * ((-vb.p.w - vb.p.x) / (dv.p.x + dv.p.w));
							vd.p.x = -vd.p.w;
							dv = vc - vb;
							ve = vb + dv * ((-vb.p.w - vb.p.x) / (dv.p.x + dv.p.w));
							ve.p.x = -ve.p.w;
						}
						else
						{
							condition = 3;
							dv = va - vb;
							vd = vb + dv * ((-vb.p.w - vb.p.x) / (dv.p.x + dv.p.w));
							vd.p.x = -vd.p.w;
							dv = va - vc;
							ve = vc + dv * ((-vc.p.w - vc.p.x) / (dv.p.x + dv.p.w));
							ve.p.x = -ve.p.w;
						}
					}

					// update vertices
					vertices.push_back(vd);
					vertices.push_back(ve);
					int e = static_cast<int>(vertices.size()) - 1;
					int d = e - 1;
					if (condition == 1)
					{
						a = d, a_state = ComputeClipState(vd.p);
						b = e, b_state = ComputeClipState(ve.p);
					}
					else if (condition == 2)
					{
						a = d, a_state = ComputeClipState(vd.p);
						c = e, c_state = ComputeClipState(ve.p);
					}
					else
					{
						a = d, a_state = ComputeClipState(vd.p);
						tris_wait_clip.push(c); tris_wait_clip.push(d); tris_wait_clip.push(e);
					}
				}
			}
		}
	}

}