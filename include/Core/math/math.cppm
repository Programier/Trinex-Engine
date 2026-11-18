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
