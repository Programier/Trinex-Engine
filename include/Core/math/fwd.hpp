#pragma once

#include <Core/engine_types.hpp>
#include <glm/fwd.hpp>

namespace Engine
{
	template<size_t N, typename T>
	using VectorNT = glm::vec<N, T, glm::defaultp>;

	template<size_t N, typename T>
	using MatrixNT = glm::mat<N, N, T, glm::defaultp>;

	template<size_t N, typename T>
	class Box;

	using Vector1f = VectorNT<1, float_t>;
	using Vector2f = VectorNT<2, float_t>;
	using Vector3f = VectorNT<3, float_t>;
	using Vector4f = VectorNT<4, float_t>;

	using Vector1b = VectorNT<1, bool_t>;
	using Vector2b = VectorNT<2, bool_t>;
	using Vector3b = VectorNT<3, bool_t>;
	using Vector4b = VectorNT<4, bool_t>;

	using Vector1i = VectorNT<1, int32_t>;
	using Vector2i = VectorNT<2, int32_t>;
	using Vector3i = VectorNT<3, int32_t>;
	using Vector4i = VectorNT<4, int32_t>;

	using Vector1i32 = VectorNT<1, int32_t>;
	using Vector2i32 = VectorNT<2, int32_t>;
	using Vector3i32 = VectorNT<3, int32_t>;
	using Vector4i32 = VectorNT<4, int32_t>;

	using Vector1i16 = VectorNT<1, int16_t>;
	using Vector2i16 = VectorNT<2, int16_t>;
	using Vector3i16 = VectorNT<3, int16_t>;
	using Vector4i16 = VectorNT<4, int16_t>;

	using Vector1i8 = VectorNT<1, int8_t>;
	using Vector2i8 = VectorNT<2, int8_t>;
	using Vector3i8 = VectorNT<3, int8_t>;
	using Vector4i8 = VectorNT<4, int8_t>;

	using Vector1u = VectorNT<1, uint32_t>;
	using Vector2u = VectorNT<2, uint32_t>;
	using Vector3u = VectorNT<3, uint32_t>;
	using Vector4u = VectorNT<4, uint32_t>;

	using Vector1u32 = VectorNT<1, uint32_t>;
	using Vector2u32 = VectorNT<2, uint32_t>;
	using Vector3u32 = VectorNT<3, uint32_t>;
	using Vector4u32 = VectorNT<4, uint32_t>;

	using Vector1u16 = VectorNT<1, uint16_t>;
	using Vector2u16 = VectorNT<2, uint16_t>;
	using Vector3u16 = VectorNT<3, uint16_t>;
	using Vector4u16 = VectorNT<4, uint16_t>;

	using Vector1u8 = VectorNT<1, uint8_t>;
	using Vector2u8 = VectorNT<2, uint8_t>;
	using Vector3u8 = VectorNT<3, uint8_t>;
	using Vector4u8 = VectorNT<4, uint8_t>;

	using Matrix4f = MatrixNT<4, float_t>;
	using Matrix3f = MatrixNT<3, float_t>;
	using Matrix2f = MatrixNT<2, float_t>;

	using Matrix4i = MatrixNT<4, int32_t>;
	using Matrix3i = MatrixNT<3, int32_t>;
	using Matrix2i = MatrixNT<2, int32_t>;

	using Matrix4u = MatrixNT<4, uint32_t>;
	using Matrix3u = MatrixNT<3, uint32_t>;
	using Matrix2u = MatrixNT<2, uint32_t>;

	using Matrix4b = MatrixNT<4, bool_t>;
	using Matrix3b = MatrixNT<3, bool_t>;
	using Matrix2b = MatrixNT<2, bool_t>;

	using Quaternion = glm::quat;

	using Box2f = Box<2, float_t>;
	using Box3f = Box<3, float_t>;
	using Box2i = Box<2, int_t>;
	using Box3i = Box<3, int_t>;
}// namespace Engine
