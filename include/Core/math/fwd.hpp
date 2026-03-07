#pragma once

#include <Core/engine_types.hpp>
#include <glm/fwd.hpp>

namespace Trinex
{
	template<usize N, typename T>
	using VectorNT = glm::vec<N, T, glm::defaultp>;

	template<usize N, typename T>
	using MatrixNT = glm::mat<N, N, T, glm::defaultp>;

	template<usize C, usize R, typename T>
	using MatrixCRT = glm::mat<C, R, T, glm::defaultp>;

	template<usize N, typename T>
	class BoxNT;

	using Vector1f = VectorNT<1, f32>;
	using Vector2f = VectorNT<2, f32>;
	using Vector3f = VectorNT<3, f32>;
	using Vector4f = VectorNT<4, f32>;

	using Vector1f32 = VectorNT<1, f32>;
	using Vector2f32 = VectorNT<2, f32>;
	using Vector3f32 = VectorNT<3, f32>;
	using Vector4f32 = VectorNT<4, f32>;

	using Vector1f16 = VectorNT<1, f16>;
	using Vector2f16 = VectorNT<2, f16>;
	using Vector3f16 = VectorNT<3, f16>;
	using Vector4f16 = VectorNT<4, f16>;

	using Vector1b = VectorNT<1, bool>;
	using Vector2b = VectorNT<2, bool>;
	using Vector3b = VectorNT<3, bool>;
	using Vector4b = VectorNT<4, bool>;

	using Vector1i = VectorNT<1, i32>;
	using Vector2i = VectorNT<2, i32>;
	using Vector3i = VectorNT<3, i32>;
	using Vector4i = VectorNT<4, i32>;

	using Vector1i32 = VectorNT<1, i32>;
	using Vector2i32 = VectorNT<2, i32>;
	using Vector3i32 = VectorNT<3, i32>;
	using Vector4i32 = VectorNT<4, i32>;

	using Vector1i16 = VectorNT<1, i16>;
	using Vector2i16 = VectorNT<2, i16>;
	using Vector3i16 = VectorNT<3, i16>;
	using Vector4i16 = VectorNT<4, i16>;

	using Vector1i8 = VectorNT<1, i8>;
	using Vector2i8 = VectorNT<2, i8>;
	using Vector3i8 = VectorNT<3, i8>;
	using Vector4i8 = VectorNT<4, i8>;

	using Vector1u = VectorNT<1, u32>;
	using Vector2u = VectorNT<2, u32>;
	using Vector3u = VectorNT<3, u32>;
	using Vector4u = VectorNT<4, u32>;

	using Vector1u32 = VectorNT<1, u32>;
	using Vector2u32 = VectorNT<2, u32>;
	using Vector3u32 = VectorNT<3, u32>;
	using Vector4u32 = VectorNT<4, u32>;

	using Vector1u16 = VectorNT<1, u16>;
	using Vector2u16 = VectorNT<2, u16>;
	using Vector3u16 = VectorNT<3, u16>;
	using Vector4u16 = VectorNT<4, u16>;

	using Vector1u8 = VectorNT<1, u8>;
	using Vector2u8 = VectorNT<2, u8>;
	using Vector3u8 = VectorNT<3, u8>;
	using Vector4u8 = VectorNT<4, u8>;

	using Matrix4f = MatrixNT<4, f32>;
	using Matrix3f = MatrixNT<3, f32>;
	using Matrix2f = MatrixNT<2, f32>;

	using Matrix4x3f = MatrixCRT<4, 3, f32>;
	using Matrix3x4f = MatrixCRT<3, 4, f32>;

	using Matrix4f32 = MatrixNT<4, f32>;
	using Matrix3f32 = MatrixNT<3, f32>;
	using Matrix2f32 = MatrixNT<2, f32>;

	using Matrix4x3f32 = MatrixCRT<4, 3, f32>;
	using Matrix3x4f32 = MatrixCRT<3, 4, f32>;

	using Matrix4f16 = MatrixNT<4, f16>;
	using Matrix3f16 = MatrixNT<3, f16>;
	using Matrix2f16 = MatrixNT<2, f16>;

	using Matrix4x3f16 = MatrixCRT<4, 3, f16>;
	using Matrix3x4f16 = MatrixCRT<3, 4, f16>;

	using Matrix4i = MatrixNT<4, i32>;
	using Matrix3i = MatrixNT<3, i32>;
	using Matrix2i = MatrixNT<2, i32>;

	using Matrix4u = MatrixNT<4, u32>;
	using Matrix3u = MatrixNT<3, u32>;
	using Matrix2u = MatrixNT<2, u32>;

	using Matrix4b = MatrixNT<4, bool>;
	using Matrix3b = MatrixNT<3, bool>;
	using Matrix2b = MatrixNT<2, bool>;

	using Quaternion = glm::quat;

	using Box2f = BoxNT<2, f32>;
	using Box3f = BoxNT<3, f32>;
	using Box2i = BoxNT<2, i32>;
	using Box3i = BoxNT<3, i32>;

	struct Angle;
}// namespace Trinex
