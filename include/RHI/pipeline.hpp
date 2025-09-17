#pragma once
#include <RHI/enums.hpp>

namespace Engine
{
	class Archive;

	struct RHIDepthTest final {
		trinex_declare_struct(RHIDepthTest, void);

		RHICompareFunc func = RHICompareFunc::Less;
		bool enable         = true;
		bool write_enable   = true;

		bool serialize(Archive& ar);
	};

	struct RHIStencilTest final {
		trinex_declare_struct(RHIStencilTest, void);

		RHIStencilOp fail       = RHIStencilOp::Decr;
		RHIStencilOp depth_pass = RHIStencilOp::Decr;
		RHIStencilOp depth_fail = RHIStencilOp::Decr;
		RHICompareFunc compare  = RHICompareFunc::Less;
		byte compare_mask       = 0;
		byte write_mask         = 0;
		byte reference          = 0;
		bool enable             = false;

		bool serialize(Archive& ar);
	};

	struct RHIColorBlending final {
		trinex_declare_struct(RHIColorBlending, void);

		RHIBlendFunc src_color_func = RHIBlendFunc::SrcAlpha;
		RHIBlendFunc dst_color_func = RHIBlendFunc::OneMinusSrcAlpha;
		RHIBlendOp color_op         = RHIBlendOp::Add;
		RHIBlendFunc src_alpha_func = RHIBlendFunc::One;
		RHIBlendFunc dst_alpha_func = RHIBlendFunc::OneMinusSrcAlpha;
		RHIBlendOp alpha_op         = RHIBlendOp::Add;
		bool enable                 = false;

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
