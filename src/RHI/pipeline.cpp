#include <Core/archive.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <RHI/pipeline.hpp>

namespace Engine
{
	static constexpr auto s_flags = Refl::Property::IsTransient;

	trinex_implement_struct(Engine::RHIDepthTest, 0)
	{
		trinex_refl_prop(enable, s_flags)->tooltip("Enable depth test");
		trinex_refl_prop(write_enable, s_flags)->tooltip("Enable write to depth buffer");
		trinex_refl_prop(func, s_flags)->tooltip("Depth compare function");
	}

	trinex_implement_struct(Engine::RHIStencilTest, 0)
	{
		trinex_refl_prop(enable, s_flags)->tooltip("Enable stencil test");
		trinex_refl_prop(fail, s_flags)->tooltip("Operation on fail");
		trinex_refl_prop(depth_pass, s_flags)->tooltip("Operation on depth pass");
		trinex_refl_prop(depth_fail, s_flags)->tooltip("Operation on depth fail");
		trinex_refl_prop(compare, s_flags)->display_name("Compare func").tooltip("Stencil compare function");
		trinex_refl_prop(compare_mask, s_flags)->tooltip("Stencil compare mask");
		trinex_refl_prop(write_mask, s_flags)->tooltip("Stencil write mask");
	}

	trinex_implement_struct(Engine::RHIColorBlending, 0)
	{
		trinex_refl_prop(enable, s_flags);
		trinex_refl_prop(src_color_func, s_flags);
		trinex_refl_prop(dst_color_func, s_flags);
		trinex_refl_prop(color_op, s_flags)->display_name("Color Operator");

		trinex_refl_prop(src_alpha_func, s_flags);
		trinex_refl_prop(dst_alpha_func, s_flags);
		trinex_refl_prop(alpha_op, s_flags)->display_name("Alpha Operator");
		trinex_refl_prop(write_mask);
	}


	bool RHIDepthTest::serialize(Archive& ar)
	{
		return ar.serialize_memory(reinterpret_cast<byte*>(this), sizeof(*this));
	}

	bool RHIStencilTest::serialize(Archive& ar)
	{
		return ar.serialize(fail, depth_pass, depth_fail, compare, compare_mask, write_mask, reference, enable);
	}

	bool RHIColorBlending::serialize(Archive& ar)
	{
		return ar.serialize_memory(reinterpret_cast<byte*>(this), sizeof(*this));
	}
}// namespace Engine
