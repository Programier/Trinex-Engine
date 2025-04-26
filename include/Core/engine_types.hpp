#pragma once
#include <Core/definitions.hpp>
#include <Core/serializer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine
{
	using byte  = std::uint8_t;
	using word  = std::uint16_t;
	using dword = std::uint32_t;
	using qword = std::uint64_t;

	using bool_t   = bool;
	using short_t  = std::int16_t;
	using ushort_t = std::uint16_t;
	using int_t    = std::int32_t;
	using uint_t   = std::uint32_t;
	using int8_t   = std::int8_t;
	using uint8_t  = std::uint8_t;
	using int16_t  = std::int16_t;
	using uint16_t = std::uint16_t;
	using int32_t  = std::int32_t;
	using uint32_t = std::uint32_t;
	using int64_t  = std::int64_t;
	using uint64_t = std::uint64_t;
	using float_t  = std::float_t;
	using double_t = std::double_t;

	using signed_byte = std::int8_t;
	using size_t      = std::uint64_t;
	using ptrdiff_t   = std::int64_t;

	using Point2D  = glm::vec2;
	using Offset2D = glm::vec2;
	using Size2D   = glm::vec2;

	using Point3D      = glm::vec3;
	using Offset3D     = glm::vec3;
	using Size3D       = glm::vec3;
	using Scale3D      = glm::vec3;
	using Translate3D  = glm::vec3;
	using EulerAngle3D = glm::vec3;
	using Force        = glm::vec3;
	using LightColor   = glm::vec3;

	template<size_t N, typename T>
	using VectorNT = glm::vec<N, T, glm::defaultp>;

	using Matrix4f = glm::mat<4, 4, float_t, glm::defaultp>;
	using Matrix3f = glm::mat<3, 3, float_t, glm::defaultp>;
	using Matrix2f = glm::mat<2, 2, float_t, glm::defaultp>;

	using Matrix4i = glm::mat<4, 4, int32_t, glm::defaultp>;
	using Matrix3i = glm::mat<3, 3, int32_t, glm::defaultp>;
	using Matrix2i = glm::mat<2, 2, int32_t, glm::defaultp>;

	using Matrix4u = glm::mat<4, 4, uint32_t, glm::defaultp>;
	using Matrix3u = glm::mat<3, 3, uint32_t, glm::defaultp>;
	using Matrix2u = glm::mat<2, 2, uint32_t, glm::defaultp>;

	using Matrix4b = glm::mat<4, 4, bool_t, glm::defaultp>;
	using Matrix3b = glm::mat<3, 3, bool_t, glm::defaultp>;
	using Matrix2b = glm::mat<2, 2, bool_t, glm::defaultp>;

	using Vector1f = glm::vec<1, float_t, glm::defaultp>;
	using Vector2f = glm::vec<2, float_t, glm::defaultp>;
	using Vector3f = glm::vec<3, float_t, glm::defaultp>;
	using Vector4f = glm::vec<4, float_t, glm::defaultp>;

	using Vector1b = glm::vec<1, bool_t, glm::defaultp>;
	using Vector2b = glm::vec<2, bool_t, glm::defaultp>;
	using Vector3b = glm::vec<3, bool_t, glm::defaultp>;
	using Vector4b = glm::vec<4, bool_t, glm::defaultp>;

	using Vector1i = glm::vec<1, int32_t, glm::defaultp>;
	using Vector2i = glm::vec<2, int32_t, glm::defaultp>;
	using Vector3i = glm::vec<3, int32_t, glm::defaultp>;
	using Vector4i = glm::vec<4, int32_t, glm::defaultp>;

	using Vector1u = glm::vec<1, uint32_t, glm::defaultp>;
	using Vector2u = glm::vec<2, uint32_t, glm::defaultp>;
	using Vector3u = glm::vec<3, uint32_t, glm::defaultp>;
	using Vector4u = glm::vec<4, uint32_t, glm::defaultp>;

	using ArrayIndex          = size_t;
	using ArrayOffset         = size_t;
	using PriorityIndex       = size_t;
	using Counter             = size_t;
	using Index               = size_t;
	using MaterialLayoutIndex = size_t;
	using HashIndex           = size_t;

	using Quaternion = glm::quat;

	using BindingIndex = byte;

	using BitMask       = size_t;
	using Identifier    = std::uint64_t;
	using EnumerateType = std::uint32_t;

	namespace Refl
	{
		class Enum;
		class Struct;
		class Class;
		class Property;
		struct PropertyChangedEvent;
		class RenderPassInfo;
	}// namespace Refl
}// namespace Engine
