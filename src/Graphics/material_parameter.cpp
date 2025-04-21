#include <Core/archive.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/structures.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <cstring>

namespace Engine::MaterialParameters
{
	static Map<ShaderParameterType::Enum, Refl::Class*> s_parameter_class_map;

	template<typename T>
	static void register_parameter()
	{
		s_parameter_class_map[T::static_type()] = T::static_class_instance();
	}

	Refl::Class* Parameter::static_find_class(ShaderParameterType type)
	{
		auto it = s_parameter_class_map.find(type.value);
		if (it == s_parameter_class_map.end())
			return nullptr;
		return it->second;
	}

#define implement_parameter(name) trinex_implement_class(Engine::MaterialParameters::name, 0)

	PrimitiveBase& PrimitiveBase::update(const void* data, size_t size, ShaderParameterInfo* info)
	{
		rhi->update_scalar_parameter(data, size, info->offset, info->location);
		return *this;
	}

	bool PrimitiveBase::serialize_internal(Archive& ar, void* data, size_t size)
	{
		if (ar.is_reading())
			ar.read_data(reinterpret_cast<byte*>(data), size);
		else
			ar.write_data(reinterpret_cast<const byte*>(data), size);
		return ar;
	}

	Float4x4& Float4x4::apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info)
	{
		if (is_model)
		{
			auto matrix = component->world_transform().matrix();
			update(&matrix, sizeof(matrix), info);
		}
		else
		{
			update(&value, sizeof(Matrix4f), info);
		}
		return *this;
	}

	LocalToWorld& LocalToWorld::apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info)
	{
		auto matrix = component->proxy()->world_transform().matrix();
		rhi->update_scalar_parameter(&matrix, sizeof(matrix), info->offset, info->location);
		return *this;
	}

	Sampler::Sampler() : sampler(SamplerFilter::Point) {}

	Sampler& Sampler::apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info)
	{
		sampler.rhi_bind(info->location);
		return *this;
	}

	bool Sampler::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		return ar.serialize(sampler);
	}

	Sampler2D::Sampler2D() : sampler(SamplerFilter::Point), texture(DefaultResources::Textures::default_texture) {}

	Sampler2D& Sampler2D::apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info)
	{
		auto rhi_sampler = sampler.rhi_sampler();
		if (texture && rhi_sampler)
			texture->rhi_shader_resource_view()->bind_combined(info->location, rhi_sampler);
		return *this;
	}

	bool Sampler2D::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;

		ar.serialize(sampler);
		ar.serialize_reference(texture);
		return ar;
	}

	Texture2D::Texture2D() : texture(DefaultResources::Textures::default_texture) {}

	Texture2D& Texture2D::apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info)
	{
		if (texture)
			texture->rhi_shader_resource_view()->bind(info->location);
		return *this;
	}

	bool Texture2D::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		ar.serialize_reference(texture);
		return true;
	}

	Globals& Globals::apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info)
	{
		if (render_pass)
		{
			render_pass->scene_renderer()->bind_global_parameters(info->location);
		}
		else
		{
			warn_log("Material Parameter",
			         "The Render Pass is not valid, so the data passed to the material will be filled with zeros");

			GlobalShaderParameters params;
			std::memset(&params, 0, sizeof(params));
			rhi->update_scalar_parameter(&params, sizeof(params), 0, info->location);
		}
		return *this;
	}

	Surface::Surface() : surface(nullptr) {}

	Surface& Surface::apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info)
	{
		auto srv = surface ? surface->rhi_shader_resource_view()
		                   : DefaultResources::Textures::default_texture->rhi_shader_resource_view();
		srv->bind(info->location);
		return *this;
	}

	bool Surface::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		return ar.serialize_reference(surface);
	}

	CombinedSurface::CombinedSurface() : surface(nullptr), sampler() {}

	CombinedSurface& CombinedSurface::apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info)
	{
		RHI_ShaderResourceView* srv = surface ? surface->rhi_shader_resource_view() : nullptr;
		RHI_Sampler* rhi_sampler    = sampler.rhi_sampler();

		if (srv && rhi_sampler)
			srv->bind_combined(info->location, rhi_sampler);
		return *this;
	}

	bool CombinedSurface::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		return ar.serialize_reference(surface) && ar.serialize(sampler);
	}

	implement_parameter(Parameter) {}

	implement_parameter(Bool)
	{
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Int)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(UInt)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Bool2)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Bool3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Bool4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Int2)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Int3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Int4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(UInt2)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(UInt3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(UInt4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float2)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float3x3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float4x4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(LocalToWorld)
	{
		register_parameter<This>();
	}

	implement_parameter(Sampler)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, sampler, Refl::Property::IsTransient);
	}

	implement_parameter(Sampler2D)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, texture, Refl::Property::IsTransient);
		trinex_refl_prop(static_class_instance(), This, sampler, Refl::Property::IsTransient);
	}

	implement_parameter(Texture2D)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, texture, Refl::Property::IsTransient);
	}

	implement_parameter(Globals)
	{
		register_parameter<This>();
	}

	implement_parameter(Surface)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, surface, Refl::Property::IsTransient);
	}

	implement_parameter(CombinedSurface)
	{
		register_parameter<This>();
		trinex_refl_prop(static_class_instance(), This, surface, Refl::Property::IsTransient);
		trinex_refl_prop(static_class_instance(), This, sampler, Refl::Property::IsTransient);
	}
}// namespace Engine::MaterialParameters
