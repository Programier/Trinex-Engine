#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/matrix.hpp>
#include <Core/math/vector.hpp>
#include <Core/name.hpp>
#include <RHI/enums.hpp>

namespace Trinex
{
	class RHIContext;
	class RHIBuffer;
	class RHIRenderTargetView;
	class RHIDepthStencilView;

	struct RHIRect {
		Vector2i size;
		Vector2i pos;

		FORCE_INLINE RHIRect(Vector2i size = {0, 0}, Vector2i pos = {0, 0}) : size(size), pos(pos) {}
		FORCE_INLINE bool operator==(const RHIRect& v) const { return pos == v.pos && size == v.size; }
		FORCE_INLINE bool operator!=(const RHIRect& v) const { return !((*this) == v); }
	};

	struct RHIScissor {
		Vector2f size;
		Vector2f pos;

		FORCE_INLINE RHIScissor(Vector2f size = {1.f, 1.f}, Vector2f pos = {0.f, 0.f}) : size(size), pos(pos) {}
		FORCE_INLINE bool operator==(const RHIScissor& v) const { return pos == v.pos && size == v.size; }
		FORCE_INLINE bool operator!=(const RHIScissor& v) const { return !((*this) == v); }
	};

	struct RHIViewport {
		Vector2f size;
		Vector2f pos;
		float min_depth;
		float max_depth;

		FORCE_INLINE RHIViewport(Vector2f size = {1.f, 1.f}, Vector2f pos = {0.f, 0.f}, float min_depth = 0.f,
		                         float max_depth = 1.f)
		    : size(size), pos(pos), min_depth(min_depth), max_depth(max_depth)
		{}

		FORCE_INLINE bool operator==(const RHIViewport& v) const
		{
			return pos == v.pos && size == v.size && min_depth == v.min_depth && max_depth == v.max_depth;
		}

		FORCE_INLINE bool operator!=(const RHIViewport& v) const { return !((*this) == v); }
	};

	struct ENGINE_EXPORT RHIRange {
		u64 first = 0;
		u64 count = ~0U;
	};

	struct ENGINE_EXPORT RHIShaderParameterInfo {
		RHIShaderParameterType type = RHIShaderParameterType::Undefined;
		Name name;
		usize size   = 0;
		usize offset = 0;
		u8 binding   = 255;

		bool serialize(Archive& ar);
	};

	struct RHIVertexAttribute {
		RHIVertexSemantic semantic;
		u8 binding;

		FORCE_INLINE RHIVertexAttribute(RHIVertexSemantic semantic = RHIVertexSemantic::Position, u8 binding = 0)
		    : semantic(semantic), binding(binding)
		{}

		bool serialize(Archive& ar);
	};

	struct RHITextureRegion {
		Vector3u extent;
		Vector3u offset;
		u16 mip;
		u16 slice;

		inline RHITextureRegion(const Vector3u& extent = {0, 0, 0}, const Vector3u& offset = {0, 0, 0}, u16 mip = 0,
		                        u16 slice = 0)
		    : extent(extent), offset(offset), mip(mip), slice(slice)
		{}

		inline RHITextureRegion(const Vector2u& extent, const Vector2u& offset = {0, 0}, u16 mip = 0, u16 slice = 0)
		    : extent(extent, 1), offset(offset, 0), mip(mip), slice(slice)
		{}
	};

	struct RHITextureDescSRV {
		u16 base_slice           = 0;
		u16 slice_count          = ~0;
		u16 base_mip             = 0;
		u16 mip_count            = ~0;
		RHITextureType view_type = RHITextureType::Undefined;
	};

	struct RHITextureDescUAV {
		u16 base_slice           = 0;
		u16 slice_count          = ~0;
		u16 base_mip             = 0;
		RHITextureType view_type = RHITextureType::Undefined;
	};

	struct RHITextureDescRTV {
		u16 base_slice           = 0;
		u16 slice_count          = ~0;
		u16 base_mip             = 0;
		RHITextureType view_type = RHITextureType::Undefined;
	};

	struct RHITextureDescDSV {
		u16 base_slice           = 0;
		u16 slice_count          = ~0;
		u16 base_mip             = 0;
		RHITextureType view_type = RHITextureType::Undefined;
	};

	struct RHIRayTracingGeometryTriangles {
		RHIDeviceAddress transform     = 0;
		RHIDeviceAddress index_buffer  = 0;
		RHIDeviceAddress vertex_buffer = 0;

		RHIVertexFormat vertex_format = RHIVertexFormat::Undefined;
		RHIIndexFormat index_format   = RHIIndexFormat::UInt16;
		u64 vertex_count              = 0;
		u64 vertex_stride             = 0;
		u64 index_count               = 0;
	};

	struct RHIRayTracingGeometryAABBs {
		RHIDeviceAddress aabbs = 0;
		u64 count              = 0;
		u64 stride             = 0;
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
		u32 id : 24                                            = 0;
		u32 mask : 8                                           = 0xFF;
		u32 sbt_offset : 24                                    = 0;
		RHIRayTracingGeometryInstanceFlags flags               = RHIRayTracingGeometryInstanceFlags::Undefined;
		class RHIAccelerationStructure* acceleration_structure = nullptr;
	};

	struct RHIRayTracingAccelerationInputs {
		RHIRayTracingAccelerationLevel level = RHIRayTracingAccelerationLevel::Top;
		RHIRayTracingAccelerationFlags flags = RHIRayTracingAccelerationFlags::Undefined;

		u32 count = 0;
		union
		{
			RHIDeviceAddress instances = 0;
			const RHIRayTracingGeometry* geometries;
		};
	};

	struct ENGINE_EXPORT RHIDepthStencilState final {
		trinex_default_comparable(RHIDepthStencilState);

		struct Depth {
			trinex_default_comparable(Depth);

			RHICompareFunc::Enum func : 7;
			u8 write : 1;

			constexpr inline Depth(RHICompareFunc func, bool write) : func(func), write(write) {}
			constexpr inline bool is_enabled() const { return func != RHICompareFunc::Always; }
		} depth;

		struct Stencil {
			trinex_default_comparable(Stencil);

			RHIStencilOp::Enum fail : 4;
			RHIStencilOp::Enum depth_pass : 4;
			RHIStencilOp::Enum depth_fail : 4;
			RHICompareFunc::Enum compare : 4;
			u8 compare_mask;
			u8 write_mask;
			u8 reference;

			constexpr inline Stencil(RHICompareFunc compare = RHICompareFunc::Always, RHIStencilOp fail = RHIStencilOp::Keep,
			                         RHIStencilOp depth_fail = RHIStencilOp::Keep, RHIStencilOp depth_pass = RHIStencilOp::Keep,
			                         u8 reference = 0, u8 compare_mask = 0xFF, u8 write_mask = 0xFF)
			    : fail(fail), depth_pass(depth_pass), depth_fail(depth_fail), compare(compare), compare_mask(compare_mask),
			      write_mask(write_mask), reference(reference)
			{}

			constexpr inline bool is_enabled() const
			{
				if (compare != RHICompareFunc::Always && compare_mask != 0)
					return true;

				if ((fail | depth_pass | depth_fail) && write_mask != 0)
					return true;

				return false;
			}
		} stencil;

		constexpr inline RHIDepthStencilState(RHICompareFunc depth_func = RHICompareFunc::Always, bool depth_write = false,
		                                      RHICompareFunc stencil_compare  = RHICompareFunc::Always,
		                                      RHIStencilOp stencil_fail       = RHIStencilOp::Keep,
		                                      RHIStencilOp stencil_depth_fail = RHIStencilOp::Keep,
		                                      RHIStencilOp stencil_depth_pass = RHIStencilOp::Keep, u8 stencil_reference = 0,
		                                      u8 stencil_compare_mask = 0xFF, u8 stencil_write_mask = 0xFF)
		    : depth(depth_func, depth_write), stencil(stencil_compare, stencil_fail, stencil_depth_fail, stencil_depth_pass,
		                                              stencil_reference, stencil_compare_mask, stencil_write_mask)
		{}

		constexpr inline bool is_enabled() const { return depth.is_enabled() || stencil.is_enabled(); }
	};

	struct ENGINE_EXPORT RHIBlendingState final {
		trinex_default_comparable(RHIBlendingState);

		static constexpr RHIBlendingState opaque(RHIColorComponent write_mask = RHIColorComponent::RGBA)
		{
			return RHIBlendingState(write_mask, RHIBlendFunc::One, RHIBlendFunc::Zero, RHIBlendOp::Add, RHIBlendFunc::One,
			                        RHIBlendFunc::Zero, RHIBlendOp::Add);
		}

		static constexpr RHIBlendingState translucent(RHIColorComponent write_mask = RHIColorComponent::RGBA)
		{
			return RHIBlendingState(write_mask, RHIBlendFunc::SrcAlpha, RHIBlendFunc::OneMinusSrcAlpha, RHIBlendOp::Add,
			                        RHIBlendFunc::One, RHIBlendFunc::OneMinusSrcAlpha, RHIBlendOp::Add);
		}

		static constexpr RHIBlendingState additive(RHIColorComponent write_mask = RHIColorComponent::RGBA)
		{
			return RHIBlendingState(write_mask, RHIBlendFunc::SrcAlpha, RHIBlendFunc::One, RHIBlendOp::Add,
			                        RHIBlendFunc::SrcAlpha, RHIBlendFunc::One, RHIBlendOp::Add);
		}

		static constexpr RHIBlendingState add(RHIColorComponent write_mask = RHIColorComponent::RGBA)
		{
			return RHIBlendingState(write_mask, RHIBlendFunc::One, RHIBlendFunc::One, RHIBlendOp::Add, RHIBlendFunc::One,
			                        RHIBlendFunc::One, RHIBlendOp::Add);
		}

		static constexpr RHIBlendingState multiply(RHIColorComponent write_mask = RHIColorComponent::RGBA)
		{
			return RHIBlendingState(write_mask, RHIBlendFunc::DstColor, RHIBlendFunc::Zero, RHIBlendOp::Add,
			                        RHIBlendFunc::DstAlpha, RHIBlendFunc::Zero, RHIBlendOp::Add);
		}

		RHIColorComponent write_mask = RHIColorComponent::RGBA;
		RHIBlendFunc::Enum src_color_func : 4;
		RHIBlendFunc::Enum dst_color_func : 4;
		RHIBlendFunc::Enum src_alpha_func : 4;
		RHIBlendFunc::Enum dst_alpha_func : 4;
		RHIBlendOp::Enum color_op : 4;
		RHIBlendOp::Enum alpha_op : 4;

		constexpr inline RHIBlendingState(RHIColorComponent write_mask = RHIColorComponent::RGBA,
		                                  RHIBlendFunc src_color_func  = RHIBlendFunc::One,
		                                  RHIBlendFunc dst_color_func = RHIBlendFunc::Zero, RHIBlendOp color_op = RHIBlendOp::Add,
		                                  RHIBlendFunc src_alpha_func = RHIBlendFunc::One,
		                                  RHIBlendFunc dst_alpha_func = RHIBlendFunc::Zero, RHIBlendOp alpha_op = RHIBlendOp::Add)
		    : write_mask(write_mask), src_color_func(src_color_func), dst_color_func(dst_color_func),
		      src_alpha_func(src_alpha_func), dst_alpha_func(dst_alpha_func), color_op(color_op), alpha_op(alpha_op)
		{}

		constexpr inline bool is_enabled() const
		{
			const bool color_blend = (src_color_func != RHIBlendFunc::One) || (dst_color_func != RHIBlendFunc::Zero) ||
			                         (color_op != RHIBlendOp::Add);

			const bool alpha_blend = (src_alpha_func != RHIBlendFunc::One) || (dst_alpha_func != RHIBlendFunc::Zero) ||
			                         (alpha_op != RHIBlendOp::Add);

			return color_blend || alpha_blend;
		}

		constexpr inline bool is_opaque() const
		{
			return (src_color_func == RHIBlendFunc::One && dst_color_func == RHIBlendFunc::Zero && color_op == RHIBlendOp::Add) &&
			       (src_alpha_func == RHIBlendFunc::One && dst_alpha_func == RHIBlendFunc::Zero && alpha_op == RHIBlendOp::Add);
		}

		constexpr inline bool is_translucent() const
		{
			return (dst_color_func != RHIBlendFunc::Zero) || (dst_alpha_func != RHIBlendFunc::Zero);
		}

		constexpr inline bool is_alpha_blend() const
		{
			return src_color_func == RHIBlendFunc::SrcAlpha && dst_color_func == RHIBlendFunc::OneMinusSrcAlpha &&
			       color_op == RHIBlendOp::Add;
		}

		constexpr inline bool is_additive() const
		{
			return src_color_func == RHIBlendFunc::One && dst_color_func == RHIBlendFunc::One && color_op == RHIBlendOp::Add;
		}

		constexpr inline bool is_multiplicative() const
		{
			return (src_color_func == RHIBlendFunc::DstColor && dst_color_func == RHIBlendFunc::Zero &&
			        color_op == RHIBlendOp::Add) ||
			       (src_color_func == RHIBlendFunc::Zero && dst_color_func == RHIBlendFunc::SrcColor &&
			        color_op == RHIBlendOp::Add);
		}

		constexpr inline bool is_alpha_replace() const
		{
			return src_alpha_func == RHIBlendFunc::One && dst_alpha_func == RHIBlendFunc::Zero && alpha_op == RHIBlendOp::Add;
		}
	};

	struct RHIRasterizerState {
		trinex_default_comparable(RHIRasterizerState);

		RHICullMode::Enum cull_mode : 2       = RHICullMode::None;
		RHIPolygonMode::Enum polygon_mode : 1 = RHIPolygonMode::Fill;
		RHIFrontFace::Enum front_face : 1     = RHIFrontFace::CounterClockWise;
		u8 depth_bias : 1                     = false;
	};

	struct RHIColorAttachmentInfo {
		RHIRenderTargetView* view = nullptr;

		union
		{
			Vector4i icolor = {0, 0, 0, 0};
			Vector4u ucolor;
			Vector4f color;
		};

		RHILoadFunc load   = RHILoadFunc::Load;
		RHIStoreFunc store = RHIStoreFunc::Store;
	};

	struct RHIDepthStencilAttachmentInfo {
		RHIDepthStencilView* view = nullptr;

		float depth                = 0.f;
		u8 stencil                 = 0;
		RHILoadFunc depth_load     = RHILoadFunc::Load;
		RHILoadFunc stencil_load   = RHILoadFunc::Load;
		RHIStoreFunc depth_store   = RHIStoreFunc::Store;
		RHIStoreFunc stencil_store = RHIStoreFunc::Store;
	};

	struct RHIRenderingInfo {
		RHIColorAttachmentInfo colors[4];
		RHIDepthStencilAttachmentInfo depth_stencil;
		RHIRect rect;
		RHIRenderingFlags flags = RHIRenderingFlags::Undefined;

		RHIRenderingInfo() = default;

		inline RHIRenderingInfo(RHIDepthStencilView* dsv) { depth_stencil.view = dsv; }

		inline RHIRenderingInfo(RHIRenderTargetView* rtv0, RHIDepthStencilView* dsv = nullptr)
		{
			colors[0].view     = rtv0;
			depth_stencil.view = dsv;
		}

		inline RHIRenderingInfo(RHIRenderTargetView* rtv0, RHIRenderTargetView* rtv1, RHIDepthStencilView* dsv = nullptr)
		{
			colors[0].view     = rtv0;
			colors[1].view     = rtv1;
			depth_stencil.view = dsv;
		}

		inline RHIRenderingInfo(RHIRenderTargetView* rtv0, RHIRenderTargetView* rtv1, RHIRenderTargetView* rtv2,
		                        RHIDepthStencilView* dsv = nullptr)
		{
			colors[0].view     = rtv0;
			colors[1].view     = rtv1;
			colors[2].view     = rtv2;
			depth_stencil.view = dsv;
		}

		inline RHIRenderingInfo(RHIRenderTargetView* rtv0, RHIRenderTargetView* rtv1, RHIRenderTargetView* rtv2,
		                        RHIRenderTargetView* rtv3, RHIDepthStencilView* dsv = nullptr)
		{
			colors[0].view     = rtv0;
			colors[1].view     = rtv1;
			colors[2].view     = rtv2;
			colors[3].view     = rtv3;
			depth_stencil.view = dsv;
		}
	};

	struct RHIContextInheritanceInfo {
		RHIContext* primary              = nullptr;
		RHISurfaceFormat colors[4]       = {RHISurfaceFormat::Undefined};
		RHISurfaceFormat depth           = RHISurfaceFormat::Undefined;
		RHIContextInheritanceFlags flags = RHIContextInheritanceFlags::Undefined;
	};
}// namespace Trinex
