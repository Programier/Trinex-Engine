#pragma once
#include <Core/definitions.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine
{
	using byte  = std::uint8_t;
	using word  = std::uint16_t;
	using dword = std::uint32_t;
	using qword = std::uint64_t;

	using signed_byte = std::int8_t;
	using size_t      = std::uint64_t;
	using ptrdiff_t   = std::int64_t;

	using Point1D      = float;
	using Offset1D     = float;
	using Size1D       = float;
	using Scale1D      = float;
	using Translate1D  = float;
	using EulerAngle1D = float;
	using Distance     = float;

	using Point2D      = glm::vec2;
	using Offset2D     = glm::vec2;
	using Size2D       = glm::vec2;
	using Scale2D      = glm::vec2;
	using Translate2D  = glm::vec2;
	using EulerAngle2D = glm::vec2;

	using Point3D      = glm::vec3;
	using Offset3D     = glm::vec3;
	using Size3D       = glm::vec3;
	using Scale3D      = glm::vec3;
	using Translate3D  = glm::vec3;
	using EulerAngle3D = glm::vec3;
	using Force        = glm::vec3;
	using LightColor   = glm::vec3;

	using Color      = glm::vec4;
	using ByteColor  = glm::vec<4, byte, glm::defaultp>;
	using Color4     = glm::vec4;
	using ByteColor4 = glm::vec<4, byte, glm::defaultp>;
	using Color3     = glm::vec3;
	using ByteColor3 = glm::vec<3, byte, glm::defaultp>;

	using Matrix4f = glm::mat4;
	using Matrix3f = glm::mat3;
	using Matrix2f = glm::mat2;

	using Matrix4x2f = glm::mat4x2;
	using Matrix4x3f = glm::mat4x3;
	using Matrix4x4f = glm::mat4x4;

	using Matrix3x2f = glm::mat3x2;
	using Matrix3x3f = glm::mat3x3;
	using Matrix3x4f = glm::mat3x4;

	using Matrix2x2f = glm::mat2x2;
	using Matrix2x3f = glm::mat2x3;
	using Matrix2x4f = glm::mat2x4;


	using ArrayIndex          = size_t;
	using ArrayOffset         = size_t;
	using PriorityIndex       = size_t;
	using Counter             = size_t;
	using Index               = size_t;
	using MaterialLayoutIndex = size_t;
	using HashIndex           = size_t;


	using Quaternion = glm::quat;

	using TextureBindIndex   = byte;
	using BindingIndex       = byte;
	using TextureAttachIndex = byte;

	using AssimpObject = const void*;
	using BitMask      = size_t;
	using PixelRGB     = glm::vec<3, byte, glm::defaultp>;
	using PixelRGBA    = glm::vec<4, byte, glm::defaultp>;

	using Vector1D = glm::vec1;
	using Vector2D = glm::vec2;
	using Vector3D = glm::vec3;
	using Vector4D = glm::vec4;

	using BoolVector1D = glm::bvec1;
	using BoolVector2D = glm::bvec2;
	using BoolVector3D = glm::bvec3;
	using BoolVector4D = glm::bvec4;

	// Int Vectors
	using IntVector1D = glm::ivec1;
	using IntVector2D = glm::ivec2;
	using IntVector3D = glm::ivec3;
	using IntVector4D = glm::ivec4;

	using UIntVector1D = glm::uvec1;
	using UIntVector2D = glm::uvec2;
	using UIntVector3D = glm::uvec3;
	using UIntVector4D = glm::uvec4;

	using BufferType                = size_t;
	using FrameBufferOutputLocation = byte;
	using ColorClearValue           = Vector4D;


	using Identifier    = std::uint64_t;
	using MipMapLevel   = byte;
	using LodLevel      = float;
	using LodBias       = float;
	using EnumerateType = std::uint32_t;
	using PolicyID      = EnumerateType;

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

	using SampleMask = size_t;

	using ScriptObjectAddress = void*;

	template<size_t length, typename Type>
	using TypedVector = glm::vec<length, Type, glm::defaultp>;

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
