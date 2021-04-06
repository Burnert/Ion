#pragma once

#define GLM_FORCE_INLINE
#define GLM_FORCE_INTRINSICS
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace Ion
{
	// ----------------------------------
	// Vector Types 
	// ----------------------------------

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

	using FQuaternion = glm::fquat;
	using DQuaternion = glm::dquat;
}
