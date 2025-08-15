#pragma once
#include <Core/math/matrix.hpp>
#include <Core/math/quaternion.hpp>
#include <Core/math/vector.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>

namespace Engine::Math
{
	using glm::cross;
	using glm::distance;
	using glm::dot;
	using glm::length;
	using glm::normalize;
	using glm::reflect;
	using glm::refract;

	using glm::clamp;
	using glm::max;
	using glm::min;
	using glm::smoothstep;
	using glm::step;

	using glm::abs;
	using glm::acos;
	using glm::asin;
	using glm::atan;
	using glm::ceil;
	using glm::cos;
	using glm::degrees;
	using glm::exp;
	using glm::floor;
	using glm::fract;
	using glm::log;
	using glm::mod;
	using glm::pow;
	using glm::radians;
	using glm::sign;
	using glm::sin;
	using glm::sqrt;
	using glm::tan;

	using glm::ortho;
	using glm::perspective;

	using glm::determinant;
	using glm::inverse;
	using glm::transpose;

	using glm::angle;
	using glm::axis;
	using glm::rotate;
	using glm::slerp;

	using glm::scale;
	using glm::rotate;
	using glm::translate;

	template<typename T, glm::qualifier Q>
	GLM_FUNC_QUALIFIER glm::mat<4, 4, T, Q> look_at(glm::vec<3, T, Q> const& eye, glm::vec<3, T, Q> const& center,
	                                                glm::vec<3, T, Q> const& up)
	{
		return glm::lookAt(eye, center, up);
	}

	template<typename T, glm::qualifier Q>
	GLM_FUNC_QUALIFIER glm::qua<T, Q> quat_look_at(const glm::vec<3, T, Q>& direction, const glm::vec<3, T, Q>& up)
	{
		return glm::quatLookAt(direction, up);
	}

	template<typename genTypeT, typename genTypeU>
	GLM_FUNC_QUALIFIER genTypeT lerp(genTypeT x, genTypeT y, genTypeU a)
	{
		return glm::mix(x, y, a);
	}

	template<glm::length_t L, typename T, typename U, glm::qualifier Q>
	GLM_FUNC_QUALIFIER glm::vec<L, T, Q> lerp(glm::vec<L, T, Q> const& x, glm::vec<L, T, Q> const& y, U a)
	{
		return glm::mix(x, y, a);
	}

	template<glm::length_t L, typename T, typename U, glm::qualifier Q>
	GLM_FUNC_QUALIFIER glm::vec<L, T, Q> lerp(glm::vec<L, T, Q> const& x, glm::vec<L, T, Q> const& y, glm::vec<L, U, Q> const& a)
	{
		return glm::mix(x, y, a);
	}

	template<typename T, glm::qualifier Q>
	GLM_FUNC_DECL glm::qua<T, Q> lerp(glm::qua<T, Q> const& x, glm::qua<T, Q> const& y, T a)
	{
		return glm::mix(x, y, a);
	}

	template<typename T, glm::qualifier Q>
	GLM_FUNC_QUALIFIER glm::qua<T, Q> angle_axis(T const& angle, glm::vec<3, T, Q> const& v)
	{
		return glm::angleAxis(angle, v);
	}

	template<typename T, glm::qualifier Q>
	GLM_FUNC_QUALIFIER glm::vec<3, T, Q> euler_angles(glm::qua<T, Q> const& x)
	{
		return glm::eulerAngles(x);
	}
}// namespace Engine::Math
