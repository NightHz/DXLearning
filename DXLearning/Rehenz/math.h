#pragma once
#include <cmath>

namespace Rehenz
{
	const float pi = 3.14159265359f;

	template <typename T>
	inline T Min(T a, T b)
	{
		return (a <= b) ? a : b;
	}

	template <typename T>
	inline T Max(T a, T b)
	{
		return (a >= b) ? a : b;
	}

	// limit x to [min, max]
	template <typename T>
	inline T Clamp(T x, T min, T max)
	{
		return (x < min) ? min : ((x > max) ? max : x);
	}

	// interpolation
	template <typename T>
	inline T Lerp(T x1, T x2, float t)
	{
		return x1 + static_cast<T>((x2 - x1) * t);
	}

	// f(0)=0, f(1)=1, f(t) = 6*t^5 - 15*t^4 + 10*t^3
	inline float Fade(float t)
	{
		return t * t * t * (10 + t * (-15 + t * 6));
	}

	// smooth interpolation
	template <typename T>
	inline T Fade(T x1, T x2, float t)
	{
		return x1 + static_cast<T>((x2 - x1) * Fade(t));
	}

	// sort two number, first <= second
	template <typename T>
	inline void Sort(T& x1, T& x2)
	{
		if (x2 < x1)
		{
			T t = x1;
			x1 = x2;
			x2 = t;
		}
	}

	template <typename T>
	inline void Swap(T& x1, T& x2)
	{
		T t = x1;
		x1 = x2;
		x2 = t;
	}



	struct Matrix;
	struct Vector;
	struct Point;
	struct EulerAngles;
	struct Vector2;
	struct Vector3;

	// 4x4 matrix
	struct Matrix
	{
	public:
		float m[4][4];

		// default unit matrix
		Matrix();
		explicit Matrix(float value);
		explicit Matrix(float _00, float _01, float _02, float _03,
			float _10, float _11, float _12, float _13,
			float _20, float _21, float _22, float _23,
			float _30, float _31, float _32, float _33);

		// access date
		float& operator()(int row, int col);
		Matrix& operator=(Matrix);

		// some operator
		Matrix& operator*=(Matrix);
		Matrix& operator+=(Matrix);
		Matrix& operator-=(Matrix);
		Matrix& operator*=(float);
		Matrix& operator/=(float);

		Matrix operator-() const;

		Matrix operator*(Matrix) const;
		Matrix operator+(Matrix) const;
		Matrix operator-(Matrix) const;
		Matrix operator*(float) const;
		Matrix operator/(float) const;

		bool operator==(Matrix) const;
		bool operator!=(Matrix) const;
	};
	Matrix operator*(float, Matrix);

	// get matrix of translation
	Matrix GetMatrixT(float tx, float ty, float tz);

	// get inverse matrix of translation
	Matrix GetInverseMatrixT(float tx, float ty, float tz);

	// get matrix of translation
	Matrix GetMatrixT(Vector translation);

	// get inverse matrix of translation
	Matrix GetInverseMatrixT(Vector translation);

	// get matrix of rotation, +z -> at, +y -> up
	Matrix GetMatrixR(Vector at, Vector up);

	// get inverse matrix of rotation, +z -> at, +y -> up
	Matrix GetInverseMatrixR(Vector at, Vector up);

	Matrix GetMatrixRx(float theta);

	Matrix GetMatrixRy(float theta);

	Matrix GetMatrixRz(float theta);

	// get matrix of scale
	Matrix GetMatrixS(float scaleX, float scaleY, float scaleZ);

	// get inverse matrix of scale
	Matrix GetInverseMatrixS(float scaleX, float scaleY, float scaleZ);

	// get matrix of scale
	Matrix GetMatrixS(float scaleXYZ);

	// get inverse matrix of scale
	Matrix GetInverseMatrixS(float scaleXYZ);

	// get matrix of scale
	Matrix GetMatrixS(Vector scale);

	// get inverse matrix of scale
	Matrix GetInverseMatrixS(Vector scale);

	// get matrix of perspective, project to Cube(-1,-1,0)(1,1,1)
	Matrix GetMatrixP(float fovy, float aspect, float z_near, float z_far);

	// get origin point in perspective space
	Point GetOriginP(float z_near, float z_far);

	// get matrix of rotation defined by eular angles
	Matrix GetMatrixE(float psi, float theta, float phi);

	// get matrix of rotation defined by eular angles
	Matrix GetMatrixE(EulerAngles euler_angles);

	// get inverse matrix of rotation defined by eular angles
	Matrix GetInverseMatrixE(EulerAngles euler_angles);

	// 3-component vector with w = 1x4 matrix
	struct Vector
	{
	public:
#pragma warning(disable:4201)
		union {
			float v[4];
			struct {
				float x;
				float y;
				float z;
				float w;
			};
		};
#pragma warning(default:4201)

		// default w = 0
		Vector();
		explicit Vector(float value);
		explicit Vector(float _x, float _y, float _z, float _w = 0);

		// access date
		float& operator()(int index);
		Vector& operator=(Vector);

		// some operator
		Vector& operator*=(Matrix);
		Vector& operator+=(Vector);
		Vector& operator-=(Vector);
		Vector& operator*=(float);
		Vector& operator/=(float);

		Vector operator-() const;

		Vector operator*(Matrix) const;
		Vector operator+(Vector) const;
		Vector operator-(Vector) const;
		Vector operator*(float) const;
		Vector operator/(float) const;

		bool operator==(Vector) const;
		bool operator!=(Vector) const;
	};
	Vector operator*(float, Vector);

	// 3-component point with w = 1x4 matrix
	struct Point : public Vector
	{
	public:
		// default w = 1
		Point() : Vector() { w = 1; }
		Point(Vector v) : Vector(v) {}
		explicit Point(float value) : Vector(value) {}
		explicit Point(float _x, float _y, float _z, float _w = 1) : Vector(_x, _y, _z, _w) {}
	};

	// eular angles
	struct EulerAngles
	{
	public:
		float psi;
		float theta;
		float phi;

		EulerAngles();
		explicit EulerAngles(float _psi, float _theta, float _phi);
	};

	// 2-component vector
	struct Vector2
	{
	public:
		float x;
		float y;

		Vector2();
		Vector2(const Vector& v);
		explicit Vector2(float _x, float _y);

		// some operator
		Vector2& operator+=(Vector2);
		Vector2& operator-=(Vector2);
		Vector2& operator*=(float);
		Vector2& operator/=(float);

		Vector2 operator-() const;

		Vector2 operator+(Vector2) const;
		Vector2 operator-(Vector2) const;
		Vector2 operator*(float) const;
		Vector2 operator/(float) const;

		bool operator==(Vector2) const;
		bool operator!=(Vector2) const;
	};
	Vector2 operator*(float, Vector2);

	// 2-component point
	typedef Vector2 Point2;

	// 3-component vector without w
	struct Vector3
	{
	public:
		float x;
		float y;
		float z;

		Vector3();
		Vector3(const Vector& v);
		explicit Vector3(float _x, float _y, float _z);

		// some operator
		Vector3& operator+=(Vector3);
		Vector3& operator-=(Vector3);
		Vector3& operator*=(float);
		Vector3& operator/=(float);

		Vector3 operator-() const;

		Vector3 operator+(Vector3) const;
		Vector3 operator-(Vector3) const;
		Vector3 operator*(float) const;
		Vector3 operator/(float) const;

		bool operator==(Vector3) const;
		bool operator!=(Vector3) const;
	};
	Vector3 operator*(float, Vector3);

	// 3-component point without w
	typedef Vector3 Point3;


	// get vector length
	float VectorLength(Vector v1);

	// get vector length
	float VectorLength(Vector2 v);

	// get vector length
	float VectorLength(Vector3 v);

	// vector dot
	float VectorDot(Vector v1, Vector v2);

	// vector dot
	float VectorDot(Vector2 v1, Vector2 v2);

	// vector dot
	float VectorDot(Vector3 v1, Vector3 v2);

	// vector cross
	Vector VectorCross(Vector v1, Vector v2);

	// vector cross
	Vector3 VectorCross(Vector3 v1, Vector3 v2);

	// vector lerp
	Vector VectorLerp(Vector v1, Vector v2, float t);

	// vector normalize
	Vector VectorNormalize(Vector v1);

	// vector normalize
	Vector3 VectorNormalize(Vector3 v);

	// point lerp
	Point PointLerp(Point p1, Point p2, float t);

	// point Standardization
	Point PointStandard(Point p1, float w = 1);

	// get point distance
	float PointDistance(Point p1, Point p2);

	// get point distance
	float PointDistance(Point2 p1, Point2 p2);

	// get point distance
	float PointDistance(Point3 p1, Point3 p2);

	// get triangles normal vector
	Vector TrianglesNormal(Point p1, Point p2, Point p3);

	// 2-component vector with int type
	struct Vector2I
	{
	public:
		int x;
		int y;

		Vector2I();
		explicit Vector2I(const Vector2& v);
		explicit Vector2I(const Vector& v);
		explicit Vector2I(int _x, int _y);
	};

	// 2-component point with int type
	typedef Vector2I Point2I;
}