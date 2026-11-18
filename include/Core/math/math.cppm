module;
#define GLM_ENABLE_EXPERIMENTAL GLM_ENABLE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_ENABLE
#define GLM_FORCE_LEFT_HANDED GLM_ENABLE
#define GLM_CLIP_SPACE_FLIP_Y GLM_ENABLE

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/vector_angle.hpp>

export module Trinex.Core.Math;
export import Trinex.Core.Types;

export namespace Trinex
{
	template<usize N, typename T>
	using Vec = glm::vec<N, T, glm::defaultp>;

	template<usize C, usize R, typename T>
	using Mat = glm::mat<C, R, T, glm::defaultp>;

	// Floating point
	using Vec2 = Vec<2, f32>;
	using Vec3 = Vec<3, f32>;
	using Vec4 = Vec<4, f32>;

	using Vec2f16 = Vec<2, f16>;
	using Vec3f16 = Vec<3, f16>;
	using Vec4f16 = Vec<4, f16>;

	// Signed integers
	using Vec2i = Vec<2, i32>;
	using Vec3i = Vec<3, i32>;
	using Vec4i = Vec<4, i32>;

	using Vec2i16 = Vec<2, i16>;
	using Vec3i16 = Vec<3, i16>;
	using Vec4i16 = Vec<4, i16>;

	using Vec2i8 = Vec<2, i8>;
	using Vec3i8 = Vec<3, i8>;
	using Vec4i8 = Vec<4, i8>;

	// Unsigned integers
	using Vec2u = Vec<2, u32>;
	using Vec3u = Vec<3, u32>;
	using Vec4u = Vec<4, u32>;

	using Vec2u16 = Vec<2, u16>;
	using Vec3u16 = Vec<3, u16>;
	using Vec4u16 = Vec<4, u16>;

	using Vec2u8 = Vec<2, u8>;
	using Vec3u8 = Vec<3, u8>;
	using Vec4u8 = Vec<4, u8>;

	// Boolean
	using Vec2b = Vec<2, bool>;
	using Vec3b = Vec<3, bool>;
	using Vec4b = Vec<4, bool>;

	// Matrices
	using Mat2 = Mat<2, 2, f32>;
	using Mat3 = Mat<3, 3, f32>;
	using Mat4 = Mat<4, 4, f32>;

	using Mat4x3 = Mat<4, 3, f32>;
	using Mat3x4 = Mat<3, 4, f32>;

	// Quaternion
	using Quat = glm::quat;
} // namespace Trinex

export namespace Trinex::Math
{
	using glm::abs;
	using glm::acos;
	using glm::all;
	using glm::angle;
	using glm::any;
	using glm::asin;
	using glm::atan;
	using glm::axis;
	using glm::ceil;
	using glm::clamp;
	using glm::column;
	using glm::conjugate;
	using glm::cos;
	using glm::cross;
	using glm::degrees;
	using glm::determinant;
	using glm::distance;
	using glm::dot;
	using glm::equal;
	using glm::exp;
	using glm::floor;
	using glm::fract;
	using glm::identity;
	using glm::inverse;
	using glm::length;
	using glm::log;
	using glm::log2;
	using glm::mat3_cast;
	using glm::mat4_cast;
	using glm::max;
	using glm::min;
	using glm::mod;
	using glm::normalize;
	using glm::pow;
	using glm::quat_cast;
	using glm::radians;
	using glm::reflect;
	using glm::refract;
	using glm::rotate;
	using glm::round;
	using glm::row;
	using glm::scale;
	using glm::sign;
	using glm::sin;
	using glm::slerp;
	using glm::smoothstep;
	using glm::sqrt;
	using glm::step;
	using glm::tan;
	using glm::translate;
	using glm::transpose;
	using glm::trunc;
} // namespace Trinex::Math
