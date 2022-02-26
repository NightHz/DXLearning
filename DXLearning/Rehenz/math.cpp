#include "math.h"
#include <cmath>

namespace Rehenz
{
	Matrix::Matrix()
	{
		float* p = (float*)m;
		for (int i = 0; i < 16; i++)
			(*(p++)) = 0.0f;
		m[0][0] = 1.0f;
		m[1][1] = 1.0f;
		m[2][2] = 1.0f;
		m[3][3] = 1.0f;
	}

	Matrix::Matrix(float value)
	{
		m[0][0] = value; // for ignore C26495 warn
		float* p = (float*)m;
		for (int i = 0; i < 16; i++)
			(*(p++)) = value;
	}

	Matrix::Matrix(float _00, float _01, float _02, float _03,
		float _10, float _11, float _12, float _13,
		float _20, float _21, float _22, float _23,
		float _30, float _31, float _32, float _33)
	{
		m[0][0] = _00; m[0][1] = _01; m[0][2] = _02; m[0][3] = _03;
		m[1][0] = _10; m[1][1] = _11; m[1][2] = _12; m[1][3] = _13;
		m[2][0] = _20; m[2][1] = _21; m[2][2] = _22; m[2][3] = _23;
		m[3][0] = _30; m[3][1] = _31; m[3][2] = _32; m[3][3] = _33;
	}

	float& Matrix::operator()(int row, int col)
	{
		//	if (row < 0 || row >= 4 || col < 0 || col >= 4)
		//		return m[0][0];
		return m[row][col];
	}

	Matrix& Matrix::operator=(Matrix matrix0)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = matrix0(i, j);
		return (*this);
	}

	Matrix& Matrix::operator*=(Matrix matrix0)
	{
		Matrix result;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result(i, j) = m[i][0] * matrix0(0, j) + m[i][1] * matrix0(1, j) + m[i][2] * matrix0(2, j) + m[i][3] * matrix0(3, j);
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = result(i, j);
		return (*this);
	}

	Matrix& Matrix::operator+=(Matrix matrix0)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = m[i][j] + matrix0(i, j);
		return (*this);
	}

	Matrix& Matrix::operator-=(Matrix matrix0)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = m[i][j] - matrix0(i, j);
		return (*this);
	}

	Matrix& Matrix::operator*=(float f0)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = m[i][j] * f0;
		return (*this);
	}

	Matrix& Matrix::operator/=(float f0)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = m[i][j] / f0;
		return (*this);
	}

	Matrix Matrix::operator-() const
	{
		Matrix result;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result(i, j) = -m[i][j];
		return result;
	}

	Matrix Matrix::operator*(Matrix matrix0) const
	{
		Matrix result;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result(i, j) = m[i][0] * matrix0(0, j) + m[i][1] * matrix0(1, j) + m[i][2] * matrix0(2, j) + m[i][3] * matrix0(3, j);
		return result;
	}

	Matrix Matrix::operator+(Matrix matrix0) const
	{
		Matrix result;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result(i, j) = result(i, j) + matrix0(i, j);
		return result;
	}

	Matrix Matrix::operator-(Matrix matrix0) const
	{
		Matrix result;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result(i, j) = result(i, j) - matrix0(i, j);
		return result;
	}

	Matrix Matrix::operator*(float f0) const
	{
		Matrix result;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result(i, j) = result(i, j) * f0;
		return result;
	}

	Matrix operator*(float f0, Matrix matrix0)
	{
		Matrix result;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result(i, j) = f0 * matrix0(i, j);
		return result;
	}

	Matrix Matrix::operator/(float f0) const
	{
		Matrix result;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result(i, j) = result(i, j) / f0;
		return result;
	}

	bool Matrix::operator==(Matrix matrix0) const
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				if (m[i][j] != matrix0(i, j))
					return false;
		return true;
	}

	bool Matrix::operator!=(Matrix matrix0) const
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				if (m[i][j] != matrix0(i, j))
					return true;
		return false;
	}

	Matrix GetMatrixT(float tx, float ty, float tz)
	{
		Matrix result;
		result(3, 0) = tx;
		result(3, 1) = ty;
		result(3, 2) = tz;
		return result;
	}

	Matrix GetInverseMatrixT(float tx, float ty, float tz)
	{
		Matrix result;
		result(3, 0) = -tx;
		result(3, 1) = -ty;
		result(3, 2) = -tz;
		return result;
	}

	Matrix GetMatrixT(Vector translation)
	{
		Matrix result;
		result(3, 0) = translation.x;
		result(3, 1) = translation.y;
		result(3, 2) = translation.z;
		return result;
	}

	Matrix GetInverseMatrixT(Vector translation)
	{
		Matrix result;
		result(3, 0) = -translation.x;
		result(3, 1) = -translation.y;
		result(3, 2) = -translation.z;
		return result;
	}

	Matrix GetMatrixR(Vector at, Vector up)
	{
		Matrix result;
		Vector xaxis, yaxis, zaxis;

		zaxis = VectorNormalize(at);
		xaxis = VectorCross(up, zaxis);
		xaxis = VectorNormalize(xaxis);
		yaxis = VectorCross(zaxis, xaxis);

		// base vector as row vector
		result(0, 0) = xaxis.x;
		result(0, 1) = xaxis.y;
		result(0, 2) = xaxis.z;

		result(1, 0) = yaxis.x;
		result(1, 1) = yaxis.y;
		result(1, 2) = yaxis.z;

		result(2, 0) = zaxis.x;
		result(2, 1) = zaxis.y;
		result(2, 2) = zaxis.z;

		return result;
	}

	Matrix GetInverseMatrixR(Vector at, Vector up)
	{
		Matrix result;
		Vector xaxis, yaxis, zaxis;

		zaxis = VectorNormalize(at);
		xaxis = VectorCross(up, zaxis);
		xaxis = VectorNormalize(xaxis);
		yaxis = VectorCross(zaxis, xaxis);

		// base vector as row vector
		result(0, 0) = xaxis.x;
		result(1, 0) = xaxis.y;
		result(2, 0) = xaxis.z;

		result(0, 1) = yaxis.x;
		result(1, 1) = yaxis.y;
		result(2, 1) = yaxis.z;

		result(0, 2) = zaxis.x;
		result(1, 2) = zaxis.y;
		result(2, 2) = zaxis.z;

		return result;
	}

	Matrix GetMatrixRx(float theta)
	{
		Matrix result;
		result(1, 1) = cosf(theta);
		result(1, 2) = sinf(theta);
		result(2, 1) = -result(1, 2);
		result(2, 2) = result(1, 1);
		return result;
	}

	Matrix GetMatrixRy(float theta)
	{
		Matrix result;
		result(0, 0) = cosf(theta);
		result(2, 0) = sinf(theta);
		result(0, 2) = -result(2, 0);
		result(2, 2) = result(0, 0);
		return result;
	}

	Matrix GetMatrixRz(float theta)
	{
		Matrix result;
		result(0, 0) = cosf(theta);
		result(0, 1) = sinf(theta);
		result(1, 0) = -result(0, 1);
		result(1, 1) = result(0, 0);
		return result;
	}

	Matrix GetMatrixS(float scaleX, float scaleY, float scaleZ)
	{
		Matrix result;
		result(0, 0) = scaleX;
		result(1, 1) = scaleY;
		result(2, 2) = scaleZ;
		return result;
	}

	Matrix GetInverseMatrixS(float scaleX, float scaleY, float scaleZ)
	{
		Matrix result;
		result(0, 0) = 1 / scaleX;
		result(1, 1) = 1 / scaleY;
		result(2, 2) = 1 / scaleZ;
		return result;
	}

	Matrix GetMatrixS(float scaleXYZ)
	{
		Matrix result;
		result(0, 0) = scaleXYZ;
		result(1, 1) = scaleXYZ;
		result(2, 2) = scaleXYZ;
		return result;
	}

	Matrix GetInverseMatrixS(float scaleXYZ)
	{
		Matrix result;
		result(0, 0) = 1 / scaleXYZ;
		result(1, 1) = 1 / scaleXYZ;
		result(2, 2) = 1 / scaleXYZ;
		return result;
	}

	Matrix GetMatrixS(Vector scale)
	{
		Matrix result;
		result(0, 0) = scale.x;
		result(1, 1) = scale.y;
		result(2, 2) = scale.z;
		return result;
	}

	Matrix GetInverseMatrixS(Vector scale)
	{
		Matrix result;
		result(0, 0) = 1 / scale.x;
		result(1, 1) = 1 / scale.y;
		result(2, 2) = 1 / scale.z;
		return result;
	}

	Matrix GetMatrixP(float fovy, float aspect, float z_near, float z_far)
	{
		Matrix result(0.0f);
		float f1 = 1 / tanf(fovy * 0.5f);
		result(0, 0) = (float)(f1 / aspect);
		result(1, 1) = (float)(f1);
		result(2, 2) = z_far / (z_far - z_near);
		result(3, 2) = -z_near * z_far / (z_far - z_near);
		result(2, 3) = 1.0f;
		return result;
	}

	Point GetOriginP(float z_near, float z_far)
	{
		return Point(0, 0, -z_near * z_far / (z_far - z_near), 0);
	}

	Matrix GetMatrixE(float psi, float theta, float phi)
	{
		Matrix result;
		result = GetMatrixRy(phi) * GetMatrixRx(theta) * GetMatrixRy(psi);
		return result;
	}

	Matrix GetMatrixE(EulerAngles euler_angles)
	{
		Matrix result;
		result = GetMatrixRy(euler_angles.phi) * GetMatrixRx(euler_angles.theta) * GetMatrixRy(euler_angles.psi);
		return result;
	}

	Matrix GetInverseMatrixE(EulerAngles euler_angles)
	{
		Matrix result;
		result = GetMatrixRy(-euler_angles.psi) * GetMatrixRx(-euler_angles.theta) * GetMatrixRy(-euler_angles.phi);
		return result;
	}

	Vector::Vector()
	{
		v[0] = 0.0f;
		v[1] = 0.0f;
		v[2] = 0.0f;
		v[3] = 0.0f;
	}

	Vector::Vector(float value)
	{
		v[0] = value;
		v[1] = value;
		v[2] = value;
		v[3] = value;
	}

	Vector::Vector(float _x, float _y, float _z, float _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	float& Vector::operator()(int index)
	{
		//	if (index < 0 || index >= 4)
		//		return v[0];
		return v[index];
	}

	Vector& Vector::operator =(Vector vector0)
	{
		for (int i = 0; i < 4; i++)
			v[i] = vector0(i);
		return (*this);
	}

	Vector& Vector::operator*=(Matrix matrix0)
	{
		Vector result;
		for (int i = 0; i < 4; i++)
			result(i) = v[0] * matrix0(0, i) + v[1] * matrix0(1, i) + v[2] * matrix0(2, i) + v[3] * matrix0(3, i);
		for (int i = 0; i < 4; i++)
			v[i] = result(i);
		return (*this);
	}

	Vector& Vector::operator+=(Vector vector0)
	{
		for (int i = 0; i < 4; i++)
			v[i] = v[i] + vector0(i);
		return (*this);
	}

	Vector& Vector::operator-=(Vector vector0)
	{
		for (int i = 0; i < 4; i++)
			v[i] = v[i] - vector0(i);
		return (*this);
	}

	Vector& Vector::operator*=(float f0)
	{
		for (int i = 0; i < 4; i++)
			v[i] = v[i] * f0;
		return (*this);
	}

	Vector& Vector::operator/=(float f0)
	{
		for (int i = 0; i < 4; i++)
			v[i] = v[i] / f0;
		return (*this);
	}

	Vector Vector::operator-() const
	{
		Vector result;
		for (int i = 0; i < 4; i++)
			result(i) = -v[i];
		return result;
	}

	Vector Vector::operator*(Matrix matrix0) const
	{
		Vector result;
		for (int i = 0; i < 4; i++)
			result(i) = v[0] * matrix0(0, i) + v[1] * matrix0(1, i) + v[2] * matrix0(2, i) + v[3] * matrix0(3, i);
		return result;
	}

	Vector Vector::operator+(Vector vector0) const
	{
		Vector result;
		for (int i = 0; i < 4; i++)
			result(i) = v[i] + vector0(i);
		return result;
	}

	Vector Vector::operator-(Vector vector0) const
	{
		Vector result;
		for (int i = 0; i < 4; i++)
			result(i) = v[i] - vector0(i);
		return result;
	}

	Vector Vector::operator*(float f0) const
	{
		Vector result;
		for (int i = 0; i < 4; i++)
			result(i) = v[i] * f0;
		return result;
	}

	Vector operator*(float f0, Vector vector0)
	{
		Vector result;
		for (int i = 0; i < 4; i++)
			result(i) = f0 * vector0(i);
		return result;
	}

	Vector Vector::operator/(float f0) const
	{
		Vector result;
		for (int i = 0; i < 4; i++)
			result(i) = v[i] / f0;
		return result;
	}

	bool Vector::operator==(Vector vector0) const
	{
		for (int i = 0; i < 4; i++)
			if (v[i] != vector0(i))
				return false;
		return true;
	}

	bool Vector::operator!=(Vector vector0) const
	{
		for (int i = 0; i < 4; i++)
			if (v[i] != vector0(i))
				return true;
		return false;
	}

	EulerAngles::EulerAngles()
	{
		psi = 0;
		theta = 0;
		phi = 0;
	}

	EulerAngles::EulerAngles(float _psi, float _theta, float _phi)
	{
		psi = _psi;
		theta = _theta;
		phi = _phi;
	}

	Vector2::Vector2()
	{
		x = 0;
		y = 0;
	}

	Vector2::Vector2(const Vector& v)
	{
		x = v.x;
		y = v.y;
	}

	Vector2::Vector2(float _x, float _y)
	{
		x = _x;
		y = _y;
	}

	Vector2& Vector2::operator+=(Vector2 v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	Vector2& Vector2::operator-=(Vector2 v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	Vector2& Vector2::operator*=(float f)
	{
		x *= f;
		y *= f;
		return *this;
	}

	Vector2& Vector2::operator/=(float f)
	{
		x /= f;
		y /= f;
		return *this;
	}

	Vector2 Vector2::operator-() const
	{
		return Vector2(-x, -y);
	}

	Vector2 Vector2::operator+(Vector2 v) const
	{
		return Vector2(x + v.x, y + v.y);
	}

	Vector2 Vector2::operator-(Vector2 v) const
	{
		return Vector2(x - v.x, y - v.y);
	}

	Vector2 Vector2::operator*(float f) const
	{
		return Vector2(f * x, f * y);
	}

	Vector2 Vector2::operator/(float f) const
	{
		return Vector2(x / f, y / f);
	}

	bool Vector2::operator==(Vector2 v) const
	{
		if (v.x == x && v.y == y)
			return true;
		else
			return false;
	}

	bool Vector2::operator!=(Vector2 v) const
	{
		return !(*this == v);
	}

	Vector3::Vector3()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	Vector3::Vector3(const Vector& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	Vector3::Vector3(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3& Vector3::operator+=(Vector3 v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3& Vector3::operator-=(Vector3 v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3& Vector3::operator*=(float f)
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	Vector3& Vector3::operator/=(float f)
	{
		float f0 = 1 / f;
		x *= f0;
		y *= f0;
		z *= f0;
		return *this;
	}

	Vector3 Vector3::operator-() const
	{
		return Vector3(-x, -y, -z);
	}

	Vector3 Vector3::operator+(Vector3 v) const
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 Vector3::operator-(Vector3 v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 Vector3::operator*(float f) const
	{
		return Vector3(f * x, f * y, f * z);
	}

	Vector3 Vector3::operator/(float f) const
	{
		float f0 = 1 / f;
		return Vector3(x * f0, y * f0, z * f0);
	}

	bool Vector3::operator==(Vector3 v) const
	{
		if (v.x == x && v.y == y && v.z == z)
			return true;
		else
			return false;
	}

	bool Vector3::operator!=(Vector3 v) const
	{
		return !(*this == v);
	}

	Vector2 operator*(float f, Vector2 v)
	{
		return v * f;
	}

	Vector3 operator*(float f, Vector3 v)
	{
		return v * f;
	}

	float VectorLength(Vector v1)
	{
		return sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
	}

	float VectorLength(Vector2 v)
	{
		return sqrtf(v.x * v.x + v.y * v.y);
	}

	float VectorLength(Vector3 v)
	{
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	float VectorDot(Vector v1, Vector v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	float VectorDot(Vector2 v1, Vector2 v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	float VectorDot(Vector3 v1, Vector3 v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	Vector VectorCross(Vector v1, Vector v2)
	{
		Vector result;
		result.x = v1.y * v2.z - v1.z * v2.y;
		result.y = v1.z * v2.x - v1.x * v2.z;
		result.z = v1.x * v2.y - v1.y * v2.x;
		result.w = 0.0f;
		return result;
	}

	Vector3 VectorCross(Vector3 v1, Vector3 v2)
	{
		Vector3 result;
		result.x = v1.y * v2.z - v1.z * v2.y;
		result.y = v1.z * v2.x - v1.x * v2.z;
		result.z = v1.x * v2.y - v1.y * v2.x;
		return result;
	}

	Vector VectorLerp(Vector v1, Vector v2, float t)
	{
		Vector result;
		result.x = Lerp(v1.x, v2.x, t);
		result.y = Lerp(v1.y, v2.y, t);
		result.z = Lerp(v1.z, v2.z, t);
		result.w = Lerp(v1.w, v2.w, t);
		return result;
	}

	Vector VectorNormalize(Vector v1)
	{
		Vector result;
		float length = VectorLength(v1);
		if (length == 0)
			return v1;

		float f0 = 1 / length;
		result.x = v1.x * f0;
		result.y = v1.y * f0;
		result.z = v1.z * f0;
		result.w = 0.0f;

		return result;
	}

	Vector3 VectorNormalize(Vector3 v)
	{
		float length = VectorLength(v);
		if (length == 0)
			return v;

		return v / length;
	}

	Point PointLerp(Point p1, Point p2, float t)
	{
		Point result;
		result.x = Lerp(p1.x, p2.x, t);
		result.y = Lerp(p1.y, p2.y, t);
		result.z = Lerp(p1.z, p2.z, t);
		result.w = Lerp(p1.w, p2.w, t);
		return result;
	}

	Point PointStandard(Point p1, float w)
	{
		Point result;

		if (p1.w == 0)
			return p1;
		float f0 = w / p1.w;
		result.x = p1.x * f0;
		result.y = p1.y * f0;
		result.z = p1.z * f0;
		result.w = w;

		return result;
	}

	float PointDistance(Point p1, Point p2)
	{
		Vector v1 = p1 - p2;
		return sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
	}

	float PointDistance(Point2 p1, Point2 p2)
	{
		return sqrtf((float)((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)));
	}

	float PointDistance(Point3 p1, Point3 p2)
	{
		return VectorLength(p1 - p2);
	}

	Vector TrianglesNormal(Point p1, Point p2, Point p3)
	{
		return VectorCross(p2 - p1, p3 - p1);
	}

	Vector2I::Vector2I()
	{
		x = 0;
		y = 0;
	}

	Vector2I::Vector2I(const Vector2& v)
	{
		x = static_cast<int>(v.x);
		y = static_cast<int>(v.y);
	}

	Vector2I::Vector2I(const Vector& v)
	{
		x = static_cast<int>(v.x);
		y = static_cast<int>(v.y);
	}

	Vector2I::Vector2I(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
}