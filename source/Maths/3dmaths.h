// ******************************************************************************
// Filename:    3dmaths.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   3d maths library.
//
// Revision History:
//   Initial Revision - 11/03/06
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include <glm/vec3.hpp>
using namespace glm;

const float PI = 3.14159265358979323846f;


// Conversions
template <class T>
inline T DegToRad(const T &degrees) {
	return (degrees * PI) / 180;
}

template <class T>
inline T RadToDeg(const T &radians) {
	return (radians * 180) / PI;
}

template <class T>
inline void Swap(T &v1, T &v2) {
	T temp = v1;
	v1 = v2;
	v2 = temp;
}

class Matrix4x4 {
public:
	// Constructors
	Matrix4x4();
	Matrix4x4(float m[16]);

	// Setup matrices
	void SetXRotation(const float x);
	void SetYRotation(const float y);
	void SetZRotation(const float z);
	void SetRotation(const float x, const float y, const float z);
	void SetTranslation(float trans[3]);
	void SetTranslation(vec3 trans);
	void SetScale(vec3 scale);

	void AddTranslation(float *translation);
	void AddRotationRadians(float *angles);

	// Properties
	void GetMatrix(float* m) const;
	const float GetDeterminant() const;
	const Matrix4x4 GetNegative() const;
	const Matrix4x4 GetTranspose() const;
	const Matrix4x4 GetInverse() const;
	const Matrix4x4 GetOrthoNormal() const;

	const vec3 GetRightVector() const;
	const vec3 GetUpVector() const;
	const vec3 GetForwardVector() const;
	const vec3 GetTranslationVector() const;
	const void GetEuler(float *x, float *y, float *z) const;

	// Operations
	void LoadIdentity();
	void Negate();
	void Transpose();
	void Inverse();
	void OrthoNormalize();

	void SetValues(float m[16]);						// Set the values of the matrix
	void PostMultiply(Matrix4x4& matrix);				// Post multiple with another matrix
	void InverseTranslateVector(float *pVect);			// Translate a vector by the inverse of the translation part of this matrix.
	void InverseRotateVector(float *pVect);				// Rotate a vector by the inverse of the rotation part of this matrix.
	void SetValues_RotALL(float x, float y, float z);	// Generate rotation matrix
	void SetRotationRadians(float *angles);				// Set the Rotation matrix

	// Arithmetic
	static Matrix4x4 &Add(const Matrix4x4 &m1, const Matrix4x4 &m2, Matrix4x4 &result);
	static Matrix4x4 &Subtract(const Matrix4x4 &m1, const Matrix4x4 &m2, Matrix4x4 &result);
	static Matrix4x4 &Scale(const Matrix4x4 &m1, const float &scale, Matrix4x4 &result);
	static Matrix4x4 &Multiply(const Matrix4x4 &m1, const Matrix4x4 &m2, Matrix4x4 &result);
	static vec3 &Multiply(const Matrix4x4 &m1, const vec3 &v, vec3 &result);
	static bool equal(const Matrix4x4 &m1, const Matrix4x4 &m2);

	// Operators
	Matrix4x4 operator+(const Matrix4x4 &m) const { Matrix4x4 result; return Add(*this, m, result); }
	Matrix4x4& operator+=(const Matrix4x4 &m) { Add(*this, m, *this); return *this; }
	Matrix4x4 operator-(const Matrix4x4 &m) const { Matrix4x4 result; return Subtract(*this, m, result); }
	Matrix4x4& operator-=(const Matrix4x4 &m) { Subtract(*this, m, *this); return *this; }
	Matrix4x4 operator*(const float &sca) const { Matrix4x4 result; return Scale(*this, sca, result); }
	Matrix4x4& operator*=(const float &sca) { Scale(*this, sca, *this); return *this; }
	Matrix4x4 operator/(const float &sca) const { Matrix4x4 result; return Scale(*this, (1/sca), result); }
	Matrix4x4& operator/=(const float &sca) { Scale(*this, (1/sca), *this); return *this; }
	Matrix4x4 operator*(const Matrix4x4 &m) const { Matrix4x4 result; return Multiply(*this, m, result); }
	vec3 operator*(const vec3 &v) const { vec3 result; return Multiply(*this, v, result); }
	bool operator==(const Matrix4x4 &m) const { return equal(*this, m); };
	bool operator!=(const Matrix4x4 &m) const { return !equal(*this, m); };

public:
	float m[16];
};
