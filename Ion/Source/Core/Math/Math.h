#pragma once

#define GLM_FORCE_INLINE
#define GLM_FORCE_INTRINSICS

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_inverse.hpp"

namespace Ion
{
	// ----------------------------------
	// Vector Types 
	// ----------------------------------

	using Vector2 = glm::fvec2;
	using Vector3 = glm::fvec3;
	using Vector4 = glm::fvec4;

	using FVector2 = glm::fvec2;
	using FVector3 = glm::fvec3;
	using FVector4 = glm::fvec4;

	using DVector2 = glm::dvec2;
	using DVector3 = glm::dvec3;
	using DVector4 = glm::dvec4;

	using IVector2 = glm::ivec2;
	using IVector3 = glm::ivec3;
	using IVector4 = glm::ivec4;

	using UVector2 = glm::uvec2;
	using UVector3 = glm::uvec3;
	using UVector4 = glm::uvec4;

	using BVector2 = glm::bvec2;
	using BVector3 = glm::bvec3;
	using BVector4 = glm::bvec4;

	// ----------------------------------
	// Matrix Types 
	// ----------------------------------

	using Matrix2   = glm::fmat2;
	using Matrix2x3 = glm::fmat2x3;
	using Matrix2x4 = glm::fmat2x4;
	using Matrix3   = glm::fmat3;
	using Matrix3x2 = glm::fmat3x2;
	using Matrix3x4 = glm::fmat3x4;
	using Matrix4   = glm::fmat4;
	using Matrix4x2 = glm::fmat4x2;
	using Matrix4x3 = glm::fmat4x3;

	using FMatrix2   = glm::fmat2;
	using FMatrix2x3 = glm::fmat2x3;
	using FMatrix2x4 = glm::fmat2x4;
	using FMatrix3   = glm::fmat3;
	using FMatrix3x2 = glm::fmat3x2;
	using FMatrix3x4 = glm::fmat3x4;
	using FMatrix4   = glm::fmat4;
	using FMatrix4x2 = glm::fmat4x2;
	using FMatrix4x3 = glm::fmat4x3;

	using DMatrix2   = glm::dmat2;
	using DMatrix2x3 = glm::dmat2x3;
	using DMatrix2x4 = glm::dmat2x4;
	using DMatrix3   = glm::dmat3;
	using DMatrix3x2 = glm::dmat3x2;
	using DMatrix3x4 = glm::dmat3x4;
	using DMatrix4   = glm::dmat4;
	using DMatrix4x2 = glm::dmat4x2;
	using DMatrix4x3 = glm::dmat4x3;

	// ----------------------------------
	// Quaternion Types 
	// ----------------------------------

	using Quaternion = glm::fquat;

	using FQuaternion = glm::fquat;
	using DQuaternion = glm::dquat;

	//-----------------------------
	// Functions ------------------
	//-----------------------------

	namespace Math
	{
		// Matrix

		FORCEINLINE Matrix4 Translate(const Vector3& v)
		{
			return glm::translate(v);
		}

		FORCEINLINE Matrix4 Rotate(float angle, const Vector3& axis)
		{
			return glm::rotate(angle, axis);
		}

		FORCEINLINE Matrix4 Scale(const Vector3& v)
		{
			return glm::scale(v);
		}

		template<typename T>
		FORCEINLINE T InverseTranspose(const T& matrix)
		{
			return glm::inverseTranspose(matrix);
		}

		template<typename T>
		FORCEINLINE T AffineInverse(const T& matrix)
		{
			return glm::affineInverse(matrix);
		}

		// Vector

		

		// Quaternion

		FORCEINLINE Vector3 Euler(const Quaternion& q)
		{
			return glm::eulerAngles(q);
		}

		FORCEINLINE Vector3 Rotate(const Quaternion& q, const Vector3& v)
		{
			return glm::rotate(q, v);
		}

		FORCEINLINE Vector4 Rotate(const Quaternion& q, const Vector4& v)
		{
			return glm::rotate(q, v);
		}

		FORCEINLINE Quaternion Inverse(const Quaternion& q)
		{
			return glm::inverse(q);
		}

		FORCEINLINE Matrix4 ToMat4(const Quaternion& q)
		{
			return glm::toMat4(q);
		}

		// Projection

		template<typename T>
		FORCEINLINE Matrix4 Perspective(T fovy, T aspect, T zNear, T zFar)
		{
			return glm::perspective(fovy, aspect, zNear, zFar);
		}

		// Common

		template<typename T>
		FORCEINLINE GLM_CONSTEXPR T Min(T x, T minVal)
		{
			return glm::min(x, minVal);
		}

		template<typename T>
		FORCEINLINE GLM_CONSTEXPR T Max(T x, T maxVal)
		{
			return glm::max(x, maxVal);
		}

		template<typename T>
		FORCEINLINE GLM_CONSTEXPR T Clamp(T x, T minVal, T maxVal)
		{
			return glm::clamp(x, minVal, maxVal);
		}

		template<typename T, TEnableIfT<TNotV<TIsFloating<T>>, bool> = true>
		FORCEINLINE GLM_CONSTEXPR T Radians(T degrees)
		{
			return glm::radians(degrees);
		}

		template<typename T, TEnableIfT<TIsFloatingV<T>, bool> = true>
		FORCEINLINE constexpr T Radians(T degrees)
		{
			return degrees * static_cast<T>(0.01745329251994329576923690768489);
		}

		template<typename T, TEnableIfT<TNotV<TIsFloating<T>>, bool> = true>
		FORCEINLINE GLM_CONSTEXPR T Degrees(T radians)
		{
			return glm::degrees(radians);
		}

		template<typename T, TEnableIfT<TIsFloatingV<T>, bool> = true>
		FORCEINLINE GLM_CONSTEXPR T Degrees(T radians)
		{
			return radians * static_cast<T>(57.295779513082320876798154814105);
		}

		template<typename T>
		FORCEINLINE T Normalize(const T& x)
		{
			return glm::normalize(x);
		}
	}
}
