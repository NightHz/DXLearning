#include "noise_gen.h"
#include "math.h"
#include <random>

namespace Rehenz
{
	const unsigned int mask = ~0U;
	const unsigned int multiplier = 16807U; // 7^5
	const unsigned int addend = 0;

	unsigned int Hash(unsigned int obj)
	{
		for (int i = 0; i < 5; i++)
			obj = obj * multiplier + addend;
		return obj;
	}

	using random_engine = std::mt19937;

	void GHash(unsigned int seed, int x, float& g)
	{
		random_engine e(Hash(seed ^ Hash(x)));
		std::uniform_real_distribution<float> d(-1, 1);
		g = d(e);
	}

	float* Perlin1D(unsigned int seed, int x1, int x2, unsigned int block_size, float* noise)
	{
		Sort(x1, x2);
		float* p = noise;
		float g1, g2;
		GHash(seed, x1, g1);
		for (int x = x1; x < x2; x++)
		{
			GHash(seed, x + 1, g2);
			for (unsigned int i = 0; i < block_size; i++)
				*(p++) = Fade(g1, g2, static_cast<float>(i) / block_size);
			g1 = g2;
		}
		return noise;
	}

	PerlinNoise1D::PerlinNoise1D(unsigned int _seed)
	{
		seed = _seed;
	}

	PerlinNoise1D::~PerlinNoise1D()
	{
	}

	float PerlinNoise1D::GetGrad(int x)
	{
		if (grads.find(x) == grads.end())
		{
			random_engine e(Hash(seed ^ Hash(x)));
			std::uniform_real_distribution<float> d(-1, 1);
			grads[x] = d(e);
		}
		return grads[x];
	}

	float PerlinNoise1D::GetNoise(float x)
	{
		int xi = static_cast<int>(x);
		float xf = x - xi;
		if (xf < 0)
			xi--, xf = x - xi;
		return Fade(GetGrad(xi), GetGrad(xi + 1), xf);
	}

	Vector2 PerlinNoise2D::GetGrad(int x, int y)
	{
		Point2 p(static_cast<float>(x), static_cast<float>(y));
		if (grads.find(p) == grads.end())
		{
			random_engine e(Hash(seed ^ Hash(x ^ Hash(y))));
			std::uniform_real_distribution<float> d(-1, 1);
			while (true)
			{
				Vector2 g;
				g.x = d(e), g.y = d(e);
				float length = VectorLength(g);
				if (length < 1.0f && length > 0.1f)
				{
					grads[p] = g * (1 / length);
					break;
				}
			}
		}
		return grads[p];
	}

	PerlinNoise2D::PerlinNoise2D(unsigned int _seed) : grads()
	{
		seed = _seed;
	}

	PerlinNoise2D::~PerlinNoise2D()
	{
	}

	float PerlinNoise2D::GetNoise(float x, float y)
	{
		int xi = static_cast<int>(x), yi = static_cast<int>(y);
		float xf = x - xi, yf = y - yi;
		if (xf < 0)
			xi--, xf = x - xi;
		if (yf < 0)
			yi--, yf = y - yi;
		float n0, n1, n2, n3;
		n0 = VectorDot(GetGrad(xi, yi), Vector2(xf, yf));
		n1 = VectorDot(GetGrad(xi + 1, yi), Vector2(xf - 1, yf));
		n2 = Fade(n0, n1, xf);
		n0 = VectorDot(GetGrad(xi, yi + 1), Vector2(xf, yf - 1));
		n1 = VectorDot(GetGrad(xi + 1, yi + 1), Vector2(xf - 1, yf - 1));
		n3 = Fade(n0, n1, xf);
		return Fade(n2, n3, yf);
	}

	float PerlinNoise2D::GetNoiseSum(float x, float y)
	{
		float max = 0;
		float s = 0;
		s += GetNoise(x, y); max += 1;
		x *= 2; y *= 2; s += 0.5000f * GetNoise(x, y); max += 0.5000f;
		x *= 2; y *= 2; s += 0.2500f * GetNoise(x, y); max += 0.2500f;
		x *= 2; y *= 2; s += 0.1250f * GetNoise(x, y); max += 0.1250f;
		x *= 2; y *= 2; s += 0.0625f * GetNoise(x, y); max += 0.0625f;
		return s / max;
	}

	float PerlinNoise2D::GetNoiseSumAbs(float x, float y)
	{
		float max = 0;
		float s = 0;
		s += abs(GetNoise(x, y)); max += 1;
		x *= 2; y *= 2; s += 0.5000f * abs(GetNoise(x, y)); max += 0.5000f;
		x *= 2; y *= 2; s += 0.2500f * abs(GetNoise(x, y)); max += 0.2500f;
		x *= 2; y *= 2; s += 0.1250f * abs(GetNoise(x, y)); max += 0.1250f;
		x *= 2; y *= 2; s += 0.0625f * abs(GetNoise(x, y)); max += 0.0625f;
		return s / max;
	}

	float PerlinNoise2D::GetNoiseSumAbsSin(float x, float y)
	{
		float s = 0;
		float x0 = x;
		s += abs(GetNoise(x, y));
		x *= 2; y *= 2; s += 0.5000f * abs(GetNoise(x, y));
		x *= 2; y *= 2; s += 0.2500f * abs(GetNoise(x, y));
		x *= 2; y *= 2; s += 0.1250f * abs(GetNoise(x, y));
		x *= 2; y *= 2; s += 0.0625f * abs(GetNoise(x, y));
		s = sin(s + x0);
		return s;
	}

	bool PerlinNoise2D::Point2Compare::operator()(const Point2& lhs, const Point2& rhs) const
	{
		if (lhs.y < rhs.y)
			return true;
		else if (rhs.y < lhs.y)
			return false;
		else if (lhs.x < rhs.x)
			return true;
		else
			return false;
	}

	Vector3 PerlinNoise3D::GetGrad(int x, int y, int z)
	{
		Point3 p(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
		if (grads.find(p) == grads.end())
		{
			random_engine e(Hash(seed ^ Hash(x ^ Hash(y ^ Hash(z)))));
			std::uniform_real_distribution<float> d(-1, 1);
			while (true)
			{
				Vector3 g;
				g.x = d(e), g.y = d(e), g.z = d(e);
				float length = VectorLength(g);
				if (length < 1.0f && length > 0.1f)
				{
					grads[p] = g * (1 / length);
					break;
				}
			}
		}
		return grads[p];
	}

	PerlinNoise3D::PerlinNoise3D(unsigned int _seed)
	{
		seed = _seed;
	}

	PerlinNoise3D::~PerlinNoise3D()
	{
	}

	float PerlinNoise3D::GetNoise(float x, float y, float z)
	{
		int xi = static_cast<int>(x), yi = static_cast<int>(y), zi = static_cast<int>(z);
		float xf = x - xi, yf = y - yi, zf = z - zi;
		if (xf < 0)
			xi--, xf = x - xi;
		if (yf < 0)
			yi--, yf = y - yi;
		if (zf < 0)
			zi--, zf = z - zi;
		float n0, n1, n2, n3, n4, n5;
		n0 = VectorDot(GetGrad(xi, yi, zi), Vector3(xf, yf, zf));
		n1 = VectorDot(GetGrad(xi + 1, yi, zi), Vector3(xf - 1, yf, zf));
		n2 = Fade(n0, n1, xf);
		n0 = VectorDot(GetGrad(xi, yi + 1, zi), Vector3(xf, yf - 1, zf));
		n1 = VectorDot(GetGrad(xi + 1, yi + 1, zi), Vector3(xf - 1, yf - 1, zf));
		n3 = Fade(n0, n1, xf);
		n4 = Fade(n2, n3, yf);
		n0 = VectorDot(GetGrad(xi, yi, zi + 1), Vector3(xf, yf, zf - 1));
		n1 = VectorDot(GetGrad(xi + 1, yi, zi + 1), Vector3(xf - 1, yf, zf - 1));
		n2 = Fade(n0, n1, xf);
		n0 = VectorDot(GetGrad(xi, yi + 1, zi + 1), Vector3(xf, yf - 1, zf - 1));
		n1 = VectorDot(GetGrad(xi + 1, yi + 1, zi + 1), Vector3(xf - 1, yf - 1, zf - 1));
		n3 = Fade(n0, n1, xf);
		n5 = Fade(n2, n3, yf);
		return Fade(n4, n5, zf);
	}

	bool PerlinNoise3D::Point3Compare::operator()(const Point3& lhs, const Point3& rhs) const
	{
		if (lhs.z < rhs.z)
			return true;
		else if (rhs.z < lhs.z)
			return false;
		if (lhs.y < rhs.y)
			return true;
		else if (rhs.y < lhs.y)
			return false;
		else if (lhs.x < rhs.x)
			return true;
		else
			return false;
	}
}