#pragma once
#include <Core/math/matrix.hpp>
#include <Core/math/quaternion.hpp>
#include <Core/math/vector.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace Trinex::Math
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

	template<typename T>
	GLM_FUNC_QUALIFIER glm::mat<4, 4, T, glm::defaultp> ortho(T left, T right, T bottom, T top, T zNear, T zFar)
	{
		return glm::ortho(left, right, bottom, top, zFar, zNear);
	}

	template<typename T>
	GLM_FUNC_QUALIFIER glm::mat<4, 4, T, glm::defaultp> perspective(T fovy, T aspect, T zNear, T zFar)
	{
		return glm::perspective(fovy, aspect, zFar, zNear);
	}

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
		a = glm::max(a, b);
		return ((a = max(a, rest)), ...);
	}

	template<glm::length_t N, typename T>
	    requires(N > 0)
	FORCE_INLINE constexpr T max(const VectorNT<N, T>& values)
	{
		T m = values[0];
		for (usize i = 1; i < N; ++i)
			if (values[i] > m)
				m = values[i];
		return m;
	}

	template<typename T, typename... Ts>
	FORCE_INLINE constexpr T min(T a, T b, Ts... rest)
	{
		a = glm::min(a, b);
		return ((a = min(a, rest)), ...);
	}

	template<glm::length_t N, typename T>
	    requires(N > 0)
	FORCE_INLINE constexpr T min(const VectorNT<N, T>& values)
	{
		T m = values[0];
		for (usize i = 1; i < N; ++i)
			if (values[i] < m)
				m = values[i];
		return m;
	}

	FORCE_INLINE constexpr float cascade_split(u32 index, u32 count, float distribution_exponent = 1.2f)
	{
		float si = static_cast<float>(index) / static_cast<float>(count);
		return pow(si, distribution_exponent);
	}

	FORCE_INLINE constexpr u32 cascade_index(float split, u32 count, float distribution_exponent = 1.2f)
	{
		return static_cast<u32>(pow(split, 1.0f / distribution_exponent) * static_cast<float>(count));
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

	template<typename T = float>
	FORCE_INLINE constexpr T half_pi()
	{
		return glm::half_pi<T>();
	}

	template<typename T>
	    requires(std::is_integral_v<T>)
	FORCE_INLINE bool is_power_of_two(T value)
	{
		return value && !(value & (value - 1));
	}

	inline Vector3u dispatch_groups(Vector3u threads, Vector3u group)
	{
		return Vector3u{
		        (threads.x + group.x - 1) / group.x,
		        (threads.y + group.y - 1) / group.y,
		        (threads.z + group.z - 1) / group.z,
		};
	}
}// namespace Trinex::Math
