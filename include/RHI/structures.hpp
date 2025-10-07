#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>
#include <Core/name.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	class RHIBuffer;

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

		FORCE_INLINE float aspect() const
		{
			if (size.x == 0 || size.y == 0)
				return 1.f;
			return static_cast<float>(size.x) / static_cast<float>(size.y);
		}

		FORCE_INLINE bool operator==(const RHIViewport& v) const
		{
			return pos == v.pos && size == v.size && min_depth == v.min_depth && max_depth == v.max_depth;
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
		RHIVertexFormat format;
		RHIVertexSemantic semantic;
		byte semantic_index;
		byte binding;

		FORCE_INLINE RHIVertexAttribute(RHIVertexSemantic semantic = RHIVertexSemantic::Position, byte semantic_index = 0,
		                                byte binding = 0)
		    : semantic(semantic), semantic_index(semantic_index), binding(binding)
		{}

		bool serialize(Archive& ar);
	};

	struct RHITextureRegion {
		Vector3u extent;
		Vector3u offset;
		uint16_t mip;
		uint16_t slice;

		inline RHITextureRegion(const Vector3u& extent = {0, 0, 0}, const Vector3u& offset = {0, 0, 0}, uint16_t mip = 0,
		                        uint16_t slice = 0)
		    : extent(extent), offset(offset), mip(mip), slice(slice)
		{}

		inline RHITextureRegion(const Vector2u& extent, const Vector2u& offset = {0, 0}, uint16_t mip = 0, uint16_t slice = 0)
		    : extent(extent, 1), offset(offset, 0), mip(mip), slice(slice)
		{}
	};

	struct RHITextureDescSRV {
		uint16_t base_slice      = 0;
		uint16_t slice_count     = ~0;
		uint16_t base_mip        = 0;
		uint16_t mip_count       = ~0;
		RHITextureType view_type = RHITextureType::Undefined;
	};

	struct RHITextureDescUAV {
		uint16_t base_slice      = 0;
		uint16_t slice_count     = ~0;
		uint16_t base_mip        = 0;
		RHITextureType view_type = RHITextureType::Undefined;
	};

	struct RHITextureDescRTV {
		uint16_t base_slice      = 0;
		uint16_t slice_count     = ~0;
		uint16_t base_mip        = 0;
		RHITextureType view_type = RHITextureType::Undefined;
	};

	struct RHITextureDescDSV {
		uint16_t base_slice      = 0;
		uint16_t slice_count     = ~0;
		uint16_t base_mip        = 0;
		RHITextureType view_type = RHITextureType::Undefined;
	};

	struct RHIRayTracingGeometryTriangles {
		RHIDeviceAddress transform     = 0;
		RHIDeviceAddress index_buffer  = 0;
		RHIDeviceAddress vertex_buffer = 0;

		RHIVertexFormat vertex_format = RHIVertexFormat::Undefined;
		RHIIndexFormat index_format   = RHIIndexFormat::UInt16;
		uint64_t vertex_count         = 0;
		uint64_t vertex_stride        = 0;
		uint64_t index_count          = 0;
	};

	struct RHIRayTracingGeometryAABBs {
		RHIDeviceAddress aabbs = 0;
		uint64_t count         = 0;
		uint64_t stride        = 0;
	};

	struct RHIRayTracingGeometry {
		RHIRayTracingGeometryType type   = RHIRayTracingGeometryType::Triangles;
		RHIRayTracingGeometryFlags flags = RHIRayTracingGeometryFlags::Undefined;

		union
		{
			RHIRayTracingGeometryTriangles* triangles = nullptr;
			RHIRayTracingGeometryAABBs* aabbs;
		};
	};

	struct RHIRayTracingAccelerationInputs {
		RHIRayTracingAccelerationLevel level = RHIRayTracingAccelerationLevel::Top;
		RHIRayTracingAccelerationFlags flags = RHIRayTracingAccelerationFlags::Undefined;

		uint32_t count = 0;
		union
		{
			RHIDeviceAddress instances = 0;
			const RHIRayTracingGeometry* geometries;
		};
	};
}// namespace Engine
