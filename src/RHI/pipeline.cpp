#include <Core/archive.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <RHI/pipeline.hpp>

namespace Engine
{
	static constexpr auto s_flags = Refl::Property::IsTransient;

	trinex_implement_struct(Engine::RHIDepthTest, 0)
	{
		auto* self = static_struct_instance();

		trinex_refl_prop(self, This, enable, s_flags)->tooltip("Enable depth test");
		trinex_refl_prop(self, This, write_enable, s_flags)->tooltip("Enable write to depth buffer");
		trinex_refl_prop(self, This, func, s_flags)->tooltip("Depth compare function");
	}

	trinex_implement_struct(Engine::RHIStencilTest, 0)
	{
		auto* self = static_struct_instance();

		trinex_refl_prop(self, This, enable, s_flags)->tooltip("Enable stencil test");
		trinex_refl_prop(self, This, fail, s_flags)->tooltip("Operation on fail");
		trinex_refl_prop(self, This, depth_pass, s_flags)->tooltip("Operation on depth pass");
		trinex_refl_prop(self, This, depth_fail, s_flags)->tooltip("Operation on depth fail");
		trinex_refl_prop(self, This, compare, s_flags)->display_name("Compare func").tooltip("Stencil compare function");
		trinex_refl_prop(self, This, compare_mask, s_flags)->tooltip("Stencil compare mask");
		trinex_refl_prop(self, This, write_mask, s_flags)->tooltip("Stencil write mask");
	}

	trinex_implement_struct(Engine::RHIColorBlending, 0)
	{
		auto* self = static_struct_instance();

		trinex_refl_prop(self, This, enable, s_flags);
		trinex_refl_prop(self, This, src_color_func, s_flags);
		trinex_refl_prop(self, This, dst_color_func, s_flags);
		trinex_refl_prop(self, This, color_op, s_flags)->display_name("Color Operator");

		trinex_refl_prop(self, This, src_alpha_func, s_flags);
		trinex_refl_prop(self, This, dst_alpha_func, s_flags);
		trinex_refl_prop(self, This, alpha_op, s_flags)->display_name("Alpha Operator");
		trinex_refl_prop(self, This, write_mask);
	}


	bool RHIDepthTest::serialize(Archive& ar)
	{
		return ar.serialize_memory(reinterpret_cast<byte*>(this), sizeof(*this));
	}

	bool RHIStencilTest::serialize(Archive& ar)
	{
		return ar.serialize_memory(reinterpret_cast<byte*>(this), sizeof(*this));
	}

	bool RHIColorBlending::serialize(Archive& ar)
	{
		return ar.serialize_memory(reinterpret_cast<byte*>(this), sizeof(*this));
	}
}// namespace Engine
