#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/matrix.hpp>
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

	struct RHIScissor {
		Vector2i size = {0, 0};
		Vector2i pos  = {0, 0};

		FORCE_INLINE RHIScissor(Vector2i size = {0, 0}, Vector2i pos = {0, 0}) : size(size), pos(pos) {}
		FORCE_INLINE bool operator==(const RHIScissor& v) const { return pos == v.pos && size == v.size; }
		FORCE_INLINE bool operator!=(const RHIScissor& v) const { return !((*this) == v); }
	};

	struct RHIViewport {
		Vector2u size   = {0, 0};
		Vector2u pos    = {0, 0};
		float min_depth = 0.0f;
		float max_depth = 1.0f;

		FORCE_INLINE RHIViewport(Vector2u size = {0, 0}, Vector2u pos = {0, 0}, float min_depth = 0.f, float max_depth = 1.f)
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

	struct ENGINE_EXPORT RHIRange {
		uint64_t first = 0;
		uint64_t count = ~0U;
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
		byte binding;

		FORCE_INLINE RHIVertexAttribute(RHIVertexSemantic semantic = RHIVertexSemantic::Position, byte binding = 0)
		    : semantic(semantic), binding(binding)
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

	struct RHIRayTracingGeometryInstance {
		Matrix4x3f transform                                   = Matrix4x3f(1.f);
		uint32_t id : 24                                       = 0;
		uint32_t mask : 8                                      = 0xFF;
		uint32_t sbt_offset : 24                               = 0;
		RHIRayTracingGeometryInstanceFlags flags               = RHIRayTracingGeometryInstanceFlags::Undefined;
		class RHIAccelerationStructure* acceleration_structure = nullptr;
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

	struct RHIDepthState final {
		trinex_declare_struct(RHIDepthState, void);
		trinex_default_comparable(RHIDepthState);

		RHICompareFunc func = RHICompareFunc::Less;
		bool enable         = true;
		bool write_enable   = true;

		RHIDepthState(bool enable = true, RHICompareFunc func = RHICompareFunc::Less, bool write_enable = true)
		    : func(func), enable(enable), write_enable(write_enable)
		{}

		bool serialize(Archive& ar);
	};

	struct RHIStencilState final {
		trinex_declare_struct(RHIStencilState, void);
		trinex_default_comparable(RHIStencilState);

		RHIStencilOp fail;
		RHIStencilOp depth_pass;
		RHIStencilOp depth_fail;
		RHICompareFunc compare;
		byte compare_mask;
		byte write_mask;
		byte reference;
		bool enable;

		inline RHIStencilState(bool enable = false, RHICompareFunc compare = RHICompareFunc::Always,
		                       RHIStencilOp fail = RHIStencilOp::Keep, RHIStencilOp depth_fail = RHIStencilOp::Keep,
		                       RHIStencilOp depth_pass = RHIStencilOp::Keep, byte reference = 0, byte compare_mask = 0xFF,
		                       byte write_mask = 0xFF)
		    : fail(fail), depth_pass(depth_pass), depth_fail(depth_fail), compare(compare), compare_mask(compare_mask),
		      write_mask(write_mask), reference(reference), enable(enable)
		{}

		bool serialize(Archive& ar);
	};

	struct ENGINE_EXPORT RHIBlendingState final {
		trinex_declare_struct(RHIBlendingState, void);
		trinex_default_comparable(RHIBlendingState);

		static RHIBlendingState opaque;
		static RHIBlendingState translucent;
		static RHIBlendingState additive;
		static RHIBlendingState add;
		static RHIBlendingState multiply;

		RHIBlendFunc src_color_func;
		RHIBlendFunc dst_color_func;
		RHIBlendOp color_op;
		RHIBlendFunc src_alpha_func;
		RHIBlendFunc dst_alpha_func;
		RHIBlendOp alpha_op;
		bool enable;

		inline RHIBlendingState(bool enable = false, RHIBlendFunc src_color_func = RHIBlendFunc::SrcAlpha,
		                        RHIBlendFunc dst_color_func = RHIBlendFunc::OneMinusSrcAlpha,
		                        RHIBlendOp color_op = RHIBlendOp::Add, RHIBlendFunc src_alpha_func = RHIBlendFunc::One,
		                        RHIBlendFunc dst_alpha_func = RHIBlendFunc::OneMinusSrcAlpha,
		                        RHIBlendOp alpha_op         = RHIBlendOp::Add)
		    : src_color_func(src_color_func), dst_color_func(dst_color_func), color_op(color_op), src_alpha_func(src_alpha_func),
		      dst_alpha_func(dst_alpha_func), alpha_op(alpha_op), enable(enable)
		{}

		bool serialize(Archive& ar);

		inline bool is_opaque() const
		{
			return !enable ||
			       (src_color_func == RHIBlendFunc::One && dst_color_func == RHIBlendFunc::Zero && color_op == RHIBlendOp::Add);
		}

		inline bool is_translucent() const { return !is_opaque(); }

		inline bool is_alpha_blend() const
		{
			return enable && src_color_func == RHIBlendFunc::SrcAlpha && dst_color_func == RHIBlendFunc::OneMinusSrcAlpha &&
			       color_op == RHIBlendOp::Add;
		}

		inline bool is_additive() const
		{
			return enable && src_color_func == RHIBlendFunc::One && dst_color_func == RHIBlendFunc::One &&
			       color_op == RHIBlendOp::Add;
		}

		inline bool is_multiplicative() const
		{
			return enable && src_color_func == RHIBlendFunc::DstColor && dst_color_func == RHIBlendFunc::Zero &&
			       color_op == RHIBlendOp::Add;
		}

		inline bool is_alpha_replace() const
		{
			return enable && src_alpha_func == RHIBlendFunc::One && dst_alpha_func == RHIBlendFunc::Zero &&
			       alpha_op == RHIBlendOp::Add;
		}

		inline bool is_symmetric() const
		{
			return src_color_func == src_alpha_func && dst_color_func == dst_alpha_func && color_op == alpha_op;
		}

		inline bool affects_color() const
		{
			return enable && !(src_color_func == RHIBlendFunc::One && dst_color_func == RHIBlendFunc::Zero);
		}
	};
}// namespace Engine
