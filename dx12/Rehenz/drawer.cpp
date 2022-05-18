#include "drawer.h"
#include "util.h"
#include <cassert>

namespace Rehenz
{
	DrawerBase::DrawerBase(uint* _buffer, int _width, int _height)
		: buffer(_buffer), w(_width), h(_height)
	{
	}

	DrawerBase::~DrawerBase()
	{
	}

	uint DrawerBase::white = ColorRGB(255, 255, 255);
	uint DrawerBase::black = ColorRGB(0, 0, 0);
	uint DrawerBase::red = ColorRGB(255, 0, 0);
	uint DrawerBase::green = ColorRGB(0, 255, 0);
	uint DrawerBase::blue = ColorRGB(0, 0, 255);
	uint DrawerBase::yellow = ColorRGB(255, 255, 0);
	uint DrawerBase::magenta = ColorRGB(255, 0, 255);
	uint DrawerBase::cyan = ColorRGB(0, 255, 255);

	uint DrawerBase::red_l = ColorRGB(242, 129, 128);
	uint DrawerBase::yellow_l = ColorRGB(223, 218, 129);
	uint DrawerBase::blue_l = ColorRGB(141, 219, 254);
	uint DrawerBase::green_l = ColorRGB(149, 224, 129);
	uint DrawerBase::purple_l = ColorRGB(141, 133, 253);
	uint DrawerBase::pink_l = ColorRGB(231, 162, 244);
	uint DrawerBase::orange_l = ColorRGB(255, 178, 125);

	void DrawerBase::Fill(uint color)
	{
		int s = w * h;
		for (int i = 0; i < s; i++)
			buffer[i] = color;
	}

	void DrawerBase::Pixel(Point2I p, uint color)
	{
		assert(p.x >= 0 && p.x < w&& p.y >= 0 && p.y < h);
		int i = p.y * w + p.x;
		buffer[i] = color;
	}

	void Drawer::Trapezoid(int y, int dy, int x1, int dx1, int dy1, int x2, int dx2, int dy2, uint color)
	{
		int x1_offset = dy1 / 2;
		int x2_offset = dy2 / 2;
		Trapezoid(y, dy, x1, x1_offset, dx1, dy1, x2, x2_offset, dx2, dy2, color);
	}

	void Drawer::Trapezoid(int y, int dy, int& x1, int& x1_offset, int dx1, int dy1,
		int& x2, int& x2_offset, int dx2, int dy2, uint color)
	{
		int y2 = y + dy;
		for (; y <= y2; y++)
		{
			while (x1_offset >= dy1)
			{
				x1_offset -= dy1;
				x1++;
			}
			while (x1_offset < 0)
			{
				x1_offset += dy1;
				x1--;
			}
			while (x2_offset >= dy2)
			{
				x2_offset -= dy2;
				x2++;
			}
			while (x2_offset < 0)
			{
				x2_offset += dy2;
				x2--;
			}
			for (int x = x1; x <= x2; x++)
				Pixel(Point2I(x, y), color);
			x1_offset += dx1;
			x2_offset += dx2;
		}
	}

	Drawer::Drawer(uint* _buffer, int _width, int _height)
		: DrawerBase(_buffer, _width, _height)
	{
	}

	Drawer::~Drawer()
	{
	}

	void Drawer::Line(Point2I p1, Point2I p2, uint color)
	{
		if (p1.x == p2.x && p1.y == p2.y)
			Pixel(p1, color);
		else if (p1.y == p2.y)
		{
			if (p2.x < p1.x)
				std::swap(p1, p2);
			for (int x = p1.x; x <= p2.x; x++)
				Pixel(Point2I(x, p1.y), color);
		}
		else if (p1.x == p2.x)
		{
			if (p2.y < p1.y)
				std::swap(p1, p2);
			for (int y = p1.y; y <= p2.y; y++)
				Pixel(Point2I(p1.x, y), color);
		}
		else
		{
			int dx = abs(p1.x - p2.x), dy = abs(p1.y - p2.y);
			if (dx >= dy)
			{
				if (p2.x < p1.x)
					std::swap(p1, p2);
				int y_offset = dx / 2; // dx times of 1/2
				if (p2.y >= p1.y)
				{
					for (auto p = p1; p.x <= p2.x; p.x++)
					{
						Pixel(p, color);
						y_offset += dy;     // dx times of dy/dx
						if (y_offset >= dx) // dx times of >= 1
						{
							y_offset -= dx; // dx times of -= 1
							p.y++;
						}
					}
				}
				else
				{
					for (auto p = p1; p.x <= p2.x; p.x++)
					{
						Pixel(p, color);
						y_offset -= dy;     // dx times of dy/dx
						if (y_offset < 0)   // dx times of < 0
						{
							y_offset += dx; // dx times of += 1
							p.y--;
						}
					}
				}
			}
			else
			{
				if (p2.y < p1.y)
					std::swap(p1, p2);
				int x_offset = dy / 2;
				if (p2.x >= p1.x)
				{
					for (auto p = p1; p.y <= p2.y; p.y++)
					{
						Pixel(p, color);
						x_offset += dx;
						if (x_offset >= dy)
						{
							x_offset -= dy;
							p.x++;
						}
					}
				}
				else
				{
					for (auto p = p1; p.y <= p2.y; p.y++)
					{
						Pixel(p, color);
						x_offset -= dx;
						if (x_offset < 0)
						{
							x_offset += dy;
							p.x--;
						}
					}
				}
			}
		}
	}

	void Drawer::Triangle(Point2I p1, Point2I p2, Point2I p3, uint color)
	{
		if (p3.y < p2.y)
			std::swap(p2, p3);
		if (p2.y < p1.y)
			std::swap(p1, p2);
		if (p3.y < p2.y)
			std::swap(p2, p3);
		// now p1.y <= p2.y <= p3.y
		if (p1.y == p3.y)
		{
			int xmin = Min(p1.x, p2.x, p3.x);
			int xmax = Max(p1.x, p2.x, p3.x);
			for (int x = xmin; x <= xmax; x++)
				Pixel(Point2I(x, p1.y), color);
		}
		else if (p1.y == p2.y)
		{
			Sort(p1.x, p2.x);
			int dy = p3.y - p1.y;
			int dx13 = p3.x - p1.x;
			int dx23 = p3.x - p2.x;
			Trapezoid(p1.y, dy, p1.x, dx13, dy, p2.x, dx23, dy, color);
			Line(p1, p3, color);
			Line(p2, p3, color);
		}
		else if (p2.y == p3.y)
		{
			Sort(p2.x, p3.x);
			int dy = p3.y - p1.y;
			int dx12 = p2.x - p1.x;
			int dx13 = p3.x - p1.x;
			Trapezoid(p1.y, dy, p1.x, dx12, dy, p1.x, dx13, dy, color);
			Line(p1, p2, color);
			Line(p1, p3, color);
		}
		else
		{
			int dy12 = p2.y - p1.y;
			int dy23 = p3.y - p2.y;
			int dy13 = p3.y - p1.y;
			int dx12 = p2.x - p1.x;
			int dx23 = p3.x - p2.x;
			int dx13 = p3.x - p1.x;
			if (dx12 * dy13 <= dy12 * dx13)
			{
				// line12 is to the left of line13
				int x12_offset = dy12 / 2;
				int x13_offset = dy13 / 2;
				int x12 = p1.x;
				int x13 = p1.x;
				Trapezoid(p1.y, dy12 - 1, x12, x12_offset, dx12, dy12, x13, x13_offset, dx13, dy13, color);
				int x23_offset = dy23 / 2;
				int x23 = p2.x;
				Trapezoid(p2.y, dy23, x23, x23_offset, dx23, dy23, x13, x13_offset, dx13, dy13, color);
			}
			else
			{
				// line12 is to the right of line13
				int x12_offset = dy12 / 2;
				int x13_offset = dy13 / 2;
				int x12 = p1.x;
				int x13 = p1.x;
				Trapezoid(p1.y, dy12 - 1, x13, x13_offset, dx13, dy13, x12, x12_offset, dx12, dy12, color);
				int x23_offset = dy23 / 2;
				int x23 = p2.x;
				Trapezoid(p2.y, dy23, x13, x13_offset, dx13, dy13, x23, x23_offset, dx23, dy23, color);
			}
			Line(p1, p2, color);
			Line(p1, p3, color);
			Line(p2, p3, color);
		}
	}

	void Drawer::Rectangle(Point2I p1, Point2I p2, uint color)
	{
		Sort(p1.x, p2.x);
		Sort(p1.y, p2.y);
		for (int y = p1.y; y <= p2.y; y++)
		{
			for (int x = p1.x; x <= p2.x; x++)
				Pixel(Point2I(x, y), color);
		}
	}

	void DrawerF::Trapezoid(float& y, float y_bottom, float& x1, float a1, float& x2, float a2, uint color)
	{
		for (; y < y_bottom; y++)
		{
			for (float x = NextHalf(x1); x < x2; x += 1.0f)
				Pixel(Point2I(static_cast<int>(x), static_cast<int>(y)), color);
			x1 += a1;
			x2 += a2;
		}
	}

	DrawerF::DrawerF(uint* _buffer, int _width, int _height)
		: DrawerBase(_buffer, _width, _height)
	{
	}

	DrawerF::~DrawerF()
	{
	}

	void DrawerF::Pixel(Point2 p, uint color)
	{
		if (p.x == w)
			p.x -= 0.1f;
		if (p.y == h)
			p.y -= 0.1f;
		Pixel(Point2I(p), color);
	}

	void DrawerF::Line(Point2 p1, Point2 p2, uint color)
	{
		if (p1.x == p2.x && p1.y == p2.y)
			Pixel(p1, color);
		else
		{
			float dx = abs(p1.x - p2.x), dy = abs(p1.y - p2.y);
			if (dx >= dy)
			{
				if (p2.x < p1.x)
					std::swap(p1, p2);
				float a = (p2.y - p1.y) / dx;
				float x = static_cast<int>(p1.x + 0.5f) + 0.5f;
				float y = p1.y + a * (x - p1.x);
				if (x - 0.5f > p1.x)
					Pixel(p1, color);
				for (; x < p2.x; x += 1.0f)
				{
					// avoid y out of bounds caused by float precision
					Point2I sp(static_cast<int>(x), static_cast<int>(y));
					if (sp.y == -1)
						sp.y++;
					else if (sp.y == h)
						sp.y--;
					Pixel(sp, color);
					y += a;
				}
				if (x - 0.5f < p2.x)
					Pixel(p2, color);
			}
			else
			{
				if (p2.y < p1.y)
					std::swap(p1, p2);
				float a = (p2.x - p1.x) / dy;
				float y = static_cast<int>(p1.y + 0.5f) + 0.5f;
				float x = p1.x + a * (y - p1.y);
				if (y - 0.5f > p1.y)
					Pixel(p1, color);
				for (; y < p2.y; y += 1.0f)
				{
					// avoid x out of bounds caused by float precision
					Point2I sp(static_cast<int>(x), static_cast<int>(y));
					if (sp.x == -1)
						sp.x++;
					else if (sp.x == w)
						sp.x--;
					Pixel(sp, color);
					x += a;
				}
				if (y - 0.5f < p2.y)
					Pixel(p2, color);
			}
		}
	}

	void DrawerF::Triangle(Point2 p1, Point2 p2, Point2 p3, uint color)
	{
		if (p3.y < p2.y)
			std::swap(p2, p3);
		if (p2.y < p1.y)
			std::swap(p1, p2);
		if (p3.y < p2.y)
			std::swap(p2, p3);
		// now p1.y <= p2.y <= p3.y
		if (p1.y == p3.y)
			return;
		else if (p1.y == p2.y)
		{
			Sort(p1.x, p2.x);
			float y = NextHalf(p1.y);
			float a13 = (p3.x - p1.x) / (p3.y - p1.y);
			float a23 = (p3.x - p2.x) / (p3.y - p2.y);
			float x13 = p1.x + a13 * (y - p1.y);
			float x23 = p2.x + a23 * (y - p2.y);
			Trapezoid(y, p3.y, x13, a13, x23, a23, color);
		}
		else if (p2.y == p3.y)
		{
			Sort(p2.x, p3.x);
			float y = NextHalf(p1.y);
			float a12 = (p2.x - p1.x) / (p2.y - p1.y);
			float a13 = (p3.x - p1.x) / (p3.y - p1.y);
			float x12 = p1.x + a12 * (y - p1.y);
			float x13 = p1.x + a13 * (y - p1.y);
			Trapezoid(y, p3.y, x12, a12, x13, a13, color);
		}
		else
		{
			float dy12 = p2.y - p1.y;
			float dy13 = p3.y - p1.y;
			float dx12 = p2.x - p1.x;
			float dx13 = p3.x - p1.x;
			if (dx12 * dy13 <= dy12 * dx13)
			{
				// line12 is to the left of line13
				float y = NextHalf(p1.y);
				float a12 = (p2.x - p1.x) / (p2.y - p1.y);
				float a13 = (p3.x - p1.x) / (p3.y - p1.y);
				float x12 = p1.x + a12 * (y - p1.y);
				float x13 = p1.x + a13 * (y - p1.y);
				Trapezoid(y, p2.y, x12, a12, x13, a13, color);
				float a23 = (p3.x - p2.x) / (p3.y - p2.y);
				float x23 = p2.x + a23 * (y - p2.y);
				Trapezoid(y, p3.y, x23, a23, x13, a13, color);
			}
			else
			{
				// line12 is to the right of line13
				float y = NextHalf(p1.y);
				float a12 = (p2.x - p1.x) / (p2.y - p1.y);
				float a13 = (p3.x - p1.x) / (p3.y - p1.y);
				float x12 = p1.x + a12 * (y - p1.y);
				float x13 = p1.x + a13 * (y - p1.y);
				Trapezoid(y, p2.y, x13, a13, x12, a12, color);
				float a23 = (p3.x - p2.x) / (p3.y - p2.y);
				float x23 = p2.x + a23 * (y - p2.y);
				Trapezoid(y, p3.y, x13, a13, x23, a23, color);
			}
		}
	}

	void DrawerV::Pixel(const Vertex& v)
	{
		assert(v.p.x >= 0 && v.p.x < w&& v.p.y >= 0 && v.p.y < h);
		int i = static_cast<int>(v.p.y) * w + static_cast<int>(v.p.x);
		if (v.p.z < zbuffer[i])
		{
			buffer[i] = ColorRGB(ps(*ps_data, VertexRecover(v)));
			zbuffer[i] = v.p.z;
		}
	}

	void DrawerV::Trapezoid(float& y, float y_bottom, Vertex& v1, const Vertex& a1, Vertex& v2, const Vertex& a2)
	{
		for (; y < y_bottom; y++)
		{
			if (v2.p.x > v1.p.x)
			{
				float x = NextHalf(v1.p.x);
				Vertex ddv = (v2 - v1) * (1.0f / (v2.p.x - v1.p.x));
				Vertex v = v1 + ddv * (x - v1.p.x);
				for (; x < v2.p.x; x += 1.0f)
				{
					Pixel(v);
					v += ddv;
				}
			}
			v1 += a1;
			v2 += a2;
		}
	}

	DrawerV::DrawerV(uint* _buffer, int _width, int _height, float* _zbuffer)
		: DrawerBase(_buffer, _width, _height), zbuffer(_zbuffer)
	{
		ps = nullptr;
		ps_data = nullptr;
	}

	DrawerV::~DrawerV()
	{
	}

	void DrawerV::FillZ(float z)
	{
		int s = w * h;
		for (int i = 0; i < s; i++)
			zbuffer[i] = z;
	}

	void DrawerV::Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, PixelShader pixel_shader, const PixelShaderData& _ps_data)
	{
		this->ps = pixel_shader;
		this->ps_data = &_ps_data;

		const Vertex* v_miny = &v1, * v_midy = &v2, * v_maxy = &v3;
		if (v_maxy->p.y < v_midy->p.y)
			std::swap(v_midy, v_maxy);
		if (v_midy->p.y < v_miny->p.y)
			std::swap(v_miny, v_midy);
		if (v_maxy->p.y < v_midy->p.y)
			std::swap(v_midy, v_maxy);

		if (v_miny->p.y == v_maxy->p.y)
			return;
		else if (v_miny->p.y == v_midy->p.y)
		{
			float y = NextHalf(v_miny->p.y);
			Vertex a13 = (*v_maxy - *v_miny) * (1.0f / (v_maxy->p.y - v_miny->p.y));
			Vertex a23 = (*v_maxy - *v_midy) * (1.0f / (v_maxy->p.y - v_midy->p.y));
			Vertex v13 = *v_miny + a13 * (y - v_miny->p.y);
			Vertex v23 = *v_midy + a23 * (y - v_midy->p.y);
			if (v_miny->p.x <= v_midy->p.x)
				Trapezoid(y, v_maxy->p.y, v13, a13, v23, a23);
			else
				Trapezoid(y, v_maxy->p.y, v23, a23, v13, a13);
		}
		else if (v_midy->p.y == v_maxy->p.y)
		{
			float y = NextHalf(v_miny->p.y);
			Vertex a12 = (*v_midy - *v_miny) * (1.0f / (v_midy->p.y - v_miny->p.y));
			Vertex a13 = (*v_maxy - *v_miny) * (1.0f / (v_maxy->p.y - v_miny->p.y));
			Vertex v12 = *v_miny + a12 * (y - v_miny->p.y);
			Vertex v13 = *v_miny + a13 * (y - v_miny->p.y);
			if (v_midy->p.x <= v_maxy->p.x)
				Trapezoid(y, v_maxy->p.y, v12, a12, v13, a13);
			else
				Trapezoid(y, v_maxy->p.y, v13, a13, v12, a12);
		}
		else
		{
			float dy12 = v_midy->p.y - v_miny->p.y;
			float dy13 = v_maxy->p.y - v_miny->p.y;
			float dx12 = v_midy->p.x - v_miny->p.x;
			float dx13 = v_maxy->p.x - v_miny->p.x;
			if (dx12 * dy13 <= dy12 * dx13)
			{
				// line12 is to the left of line13
				float y = NextHalf(v_miny->p.y);
				Vertex a12 = (*v_midy - *v_miny) * (1.0f / (v_midy->p.y - v_miny->p.y));
				Vertex a13 = (*v_maxy - *v_miny) * (1.0f / (v_maxy->p.y - v_miny->p.y));
				Vertex v12 = *v_miny + a12 * (y - v_miny->p.y);
				Vertex v13 = *v_miny + a13 * (y - v_miny->p.y);
				Trapezoid(y, v_midy->p.y, v12, a12, v13, a13);
				Vertex a23 = (*v_maxy - *v_midy) * (1.0f / (v_maxy->p.y - v_midy->p.y));
				Vertex v23 = *v_midy + a23 * (y - v_midy->p.y);
				Trapezoid(y, v_maxy->p.y, v23, a23, v13, a13);
			}
			else
			{
				// line12 is to the right of line13
				float y = NextHalf(v_miny->p.y);
				Vertex a12 = (*v_midy - *v_miny) * (1.0f / (v_midy->p.y - v_miny->p.y));
				Vertex a13 = (*v_maxy - *v_miny) * (1.0f / (v_maxy->p.y - v_miny->p.y));
				Vertex v12 = *v_miny + a12 * (y - v_miny->p.y);
				Vertex v13 = *v_miny + a13 * (y - v_miny->p.y);
				Trapezoid(y, v_midy->p.y, v13, a13, v12, a12);
				Vertex a23 = (*v_maxy - *v_midy) * (1.0f / (v_maxy->p.y - v_midy->p.y));
				Vertex v23 = *v_midy + a23 * (y - v_midy->p.y);
				Trapezoid(y, v_maxy->p.y, v13, a13, v23, a23);
			}
		}
	}

}