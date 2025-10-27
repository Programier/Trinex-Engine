#pragma once
#include <Core/math/matrix.hpp>
#include <Core/math/quaternion.hpp>
#include <Core/math/vector.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

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
	using glm::log2;
	using glm::mod;
	using glm::pow;
	using glm::radians;
	using glm::round;
	using glm::sign;
	using glm::sin;
	using glm::sqrt;
	using glm::tan;
	using glm::trunc;

	using glm::ortho;
	using glm::perspective;

	using glm::determinant;
	using glm::inverse;
	using glm::transpose;

	using glm::angle;
	using glm::axis;
	using glm::rotate;
	using glm::slerp;

	using glm::rotate;
	using glm::scale;
	using glm::translate;

	using glm::quat_cast;

	using glm::column;
	using glm::row;

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

	FORCE_INLINE constexpr float linearize_depth(float depth, float near, float far)
	{
		return (near * far) / (far + depth * (near - far));
	}

	FORCE_INLINE constexpr float unlinearize_depth(float depth, float near, float far)
	{
		return ((near * far) / depth - far) / (near - far);
	}

	template<typename T, typename... Ts>
	FORCE_INLINE constexpr T max(T a, T b, Ts... rest)
	{
		a = max(a, b);
		return ((a = max(a, rest)), ...);
	}
	template<typename T, typename... Ts>
	FORCE_INLINE constexpr T min(T a, T b, Ts... rest)
	{
		a = min(a, b);
		return ((a = min(a, rest)), ...);
	}

	FORCE_INLINE constexpr float cascade_split(uint_t index, uint_t count, float distribution_exponent = 1.2f)
	{
		float si = static_cast<float>(index) / static_cast<float>(count);
		return pow(si, distribution_exponent);
	}

	FORCE_INLINE constexpr uint_t cascade_index(float split, uint_t count, float distribution_exponent = 1.2f)
	{
		return static_cast<uint_t>(pow(split, 1.0f / distribution_exponent) * static_cast<float>(count));
	}

	FORCE_INLINE constexpr float ev100_to_luminance(float ev100)
	{
		return 12.5f * pow(2.0f, ev100);
	}

	FORCE_INLINE float luminance_to_ev100(float luminance)
	{
		return log2(luminance / 12.5f);
	}

	template<typename T = float>
	FORCE_INLINE constexpr T pi()
	{
		return glm::pi<T>();
	}

}// namespace Engine::Math
