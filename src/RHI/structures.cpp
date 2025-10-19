#include <Core/archive.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	static constexpr auto s_flags = Refl::Property::IsTransient;

	trinex_implement_struct(Engine::RHIDepthState, 0)
	{
		trinex_refl_prop(enable, s_flags)->tooltip("Enable depth test");
		trinex_refl_prop(write_enable, s_flags)->tooltip("Enable write to depth buffer");
		trinex_refl_prop(func, s_flags)->tooltip("Depth compare function");
	}

	trinex_implement_struct(Engine::RHIStencilState, 0)
	{
		trinex_refl_prop(enable, s_flags)->tooltip("Enable stencil test");
		trinex_refl_prop(fail, s_flags)->tooltip("Operation on fail");
		trinex_refl_prop(depth_pass, s_flags)->tooltip("Operation on depth pass");
		trinex_refl_prop(depth_fail, s_flags)->tooltip("Operation on depth fail");
		trinex_refl_prop(compare, s_flags)->display_name("Compare func").tooltip("Stencil compare function");
		trinex_refl_prop(compare_mask, s_flags)->tooltip("Stencil compare mask");
		trinex_refl_prop(write_mask, s_flags)->tooltip("Stencil write mask");
	}

	trinex_implement_struct(Engine::RHIBlendingState, 0)
	{
		trinex_refl_prop(enable, s_flags);
		trinex_refl_prop(src_color_func, s_flags);
		trinex_refl_prop(dst_color_func, s_flags);
		trinex_refl_prop(color_op, s_flags)->display_name("Color Operator");

		trinex_refl_prop(src_alpha_func, s_flags);
		trinex_refl_prop(dst_alpha_func, s_flags);
		trinex_refl_prop(alpha_op, s_flags)->display_name("Alpha Operator");
	}

	bool RHIDepthState::serialize(Archive& ar)
	{
		return ar.serialize(func, enable, write_enable);
	}

	bool RHIStencilState::serialize(Archive& ar)
	{
		return ar.serialize(fail, depth_pass, depth_fail, compare, compare_mask, write_mask, reference, enable);
	}


	RHIBlendingState RHIBlendingState::opaque      = RHIBlendingState(false,             //
	                                                                  RHIBlendFunc::One, //
	                                                                  RHIBlendFunc::Zero,//
	                                                                  RHIBlendOp::Add,   //
	                                                                  RHIBlendFunc::One, //
	                                                                  RHIBlendFunc::Zero,//
	                                                                  RHIBlendOp::Add);
	RHIBlendingState RHIBlendingState::translucent = RHIBlendingState(true,                          //
	                                                                  RHIBlendFunc::SrcAlpha,        //
	                                                                  RHIBlendFunc::OneMinusSrcAlpha,//
	                                                                  RHIBlendOp::Add,               //
	                                                                  RHIBlendFunc::One,             //
	                                                                  RHIBlendFunc::OneMinusSrcAlpha,//
	                                                                  RHIBlendOp::Add);
	RHIBlendingState RHIBlendingState::additive    = RHIBlendingState(true,                  //
	                                                                  RHIBlendFunc::SrcAlpha,//
	                                                                  RHIBlendFunc::One,     //
	                                                                  RHIBlendOp::Add,       //
	                                                                  RHIBlendFunc::SrcAlpha,//
	                                                                  RHIBlendFunc::One,     //
	                                                                  RHIBlendOp::Add);
	RHIBlendingState RHIBlendingState::multiply    = RHIBlendingState(true,                  //
	                                                                  RHIBlendFunc::DstColor,//
	                                                                  RHIBlendFunc::Zero,    //
	                                                                  RHIBlendOp::Add,       //
	                                                                  RHIBlendFunc::One,     //
	                                                                  RHIBlendFunc::Zero,    //
	                                                                  RHIBlendOp::Add);
	bool RHIBlendingState::serialize(Archive& ar)
	{
		return ar.serialize(src_color_func, dst_color_func, color_op, src_alpha_func, dst_alpha_func, alpha_op, enable);
	}

	bool RHIShaderParameterInfo::serialize(Archive& ar)
	{
		ar.serialize(type);
		ar.serialize(name);
		ar.serialize(size);
		ar.serialize(offset);
		ar.serialize(binding);
		return ar;
	}

	bool RHIVertexAttribute::serialize(Archive& ar)
	{
		ar.serialize(format);
		ar.serialize(semantic);
		ar.serialize(semantic_index);
		return ar.serialize(binding);
	}
}// namespace Engine
