#pragma once
#include <Core/engine_types.hpp>
#include <Core/name.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	struct RHIRect {
		Vector2i size = {0, 0};
		Vector2i pos  = {0, 0};

		FORCE_INLINE RHIRect(Vector2i size = {0, 0}, Vector2i pos = {0, 0}) : size(size), pos(pos) {}
		FORCE_INLINE bool operator==(const RHIRect& v) const { return pos == v.pos && size == v.size; }
		FORCE_INLINE bool operator!=(const RHIRect& v) const { return !((*this) == v); }
	};

	struct RHIScissors {
		Vector2i size = {0, 0};
		Vector2i pos  = {0, 0};

		FORCE_INLINE RHIScissors(Vector2i size = {0, 0}, Vector2i pos = {0, 0}) : size(size), pos(pos) {}
		FORCE_INLINE bool operator==(const RHIScissors& v) const { return pos == v.pos && size == v.size; }
		FORCE_INLINE bool operator!=(const RHIScissors& v) const { return !((*this) == v); }
	};

	struct RHIViewport {
		Vector2i size   = {0, 0};
		Vector2i pos    = {0, 0};
		float min_depth = 0.0f;
		float max_depth = 1.0f;


		FORCE_INLINE RHIViewport(Vector2i size = {0, 0}, Vector2i pos = {0, 0}, float min_depth = 0.f, float max_depth = 1.f)
		    : size(size), pos(pos), min_depth(min_depth), max_depth(max_depth)
		{}

		FORCE_INLINE float aspect() const { return static_cast<float>(size.x) / static_cast<float>(size.y); }

		FORCE_INLINE bool operator==(const RHIViewport& v) const
		{
			return pos == v.pos && size == v.size && glm::epsilonEqual(min_depth, v.min_depth, 0.0001f) &&
			       glm::epsilonEqual(max_depth, v.max_depth, 0.0001f);
		}

		FORCE_INLINE bool operator!=(const RHIViewport& v) const { return !((*this) == v); }
	};

	struct ENGINE_EXPORT RHIShaderParameterInfo {
		RHIShaderParameterType type = RHIShaderParameterType::Undefined;
		Name name;
		size_t size   = 0;
		size_t offset = 0;
		byte binding  = 255;

		bool serialize(Archive& ar);
	};

	struct RHIVertexAttribute {
		Name name;
		RHIVertexBufferElementType type;
		RHIVertexAttributeInputRate rate;
		RHIVertexBufferSemantic semantic;
		byte semantic_index;
		byte location;
		byte stream_index;
		uint16_t offset;

		FORCE_INLINE RHIVertexAttribute(RHIVertexAttributeInputRate rate = RHIVertexAttributeInputRate::Vertex,
		                                RHIVertexBufferSemantic semantic = RHIVertexBufferSemantic::Position,
		                                byte semantic_index = 0, byte location = 0, byte stream = 0, uint16_t offset = 0,
		                                const Name& name = Name::none)
		    : name(name), rate(rate), semantic(semantic), semantic_index(semantic_index), location(location),
		      stream_index(stream), offset(offset)
		{}

		bool serialize(Archive& ar);
	};

	struct RHITextureRegion {
		Vector3u extent;
		Vector3u offset;
		uint16_t mip_level;
		uint16_t array_slice;

		inline RHITextureRegion(const Vector3u& extent = {0, 0, 0}, const Vector3u& offset = {0, 0, 0}, uint16_t mip_level = 0,
		                        uint16_t array_slice = 0)
		    : extent(extent), offset(offset), mip_level(mip_level), array_slice(array_slice)
		{}

		inline RHITextureRegion(const Vector2u& extent, const Vector2u& offset = {0, 0}, uint16_t mip_level = 0,
		                        uint16_t array_slice = 0)
		    : extent(extent, 1), offset(offset, 0), mip_level(mip_level), array_slice(array_slice)
		{}
	};

	struct RHITextureDescSRV {
		uint16_t first_array_slice = 0;
		uint16_t array_size        = ~0;
		uint8_t first_mip          = 0;
		uint8_t mip_levels         = ~0;
		RHITextureType view_type   = RHITextureType::Undefined;
	};

	struct RHITextureDescUAV {
		uint16_t first_array_slice = 0;
		uint16_t array_size        = ~0;
		uint8_t mip_slice          = 0;
		RHITextureType view_type   = RHITextureType::Undefined;
	};

	struct RHITextureDescRTV {
		uint16_t first_array_slice = 0;
		uint16_t array_size        = ~0;
		uint8_t mip_slice          = 0;
		RHITextureType view_type   = RHITextureType::Undefined;
	};

	struct RHITextureDescDSV {
		uint16_t first_array_slice = 0;
		uint16_t array_size        = ~0;
		uint8_t mip_slice          = 0;
		RHITextureType view_type   = RHITextureType::Undefined;
	};
}// namespace Engine
