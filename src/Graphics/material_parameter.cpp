#include <Core/archive.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <RHI/rhi.hpp>

namespace Engine::MaterialParameters
{
	static Vector<Refl::Class*> s_parameter_classes;

	template<typename T>
	static void register_parameter()
	{
		uint16_t index = T::static_type().type_index();
		if (index >= s_parameter_classes.size())
			s_parameter_classes.resize(index + 1);
		s_parameter_classes[index] = T::static_reflection();
	}

	Refl::Class* Parameter::static_find_class(RHIShaderParameterType type)
	{
		uint16_t index = type.type_index();
		if (index >= s_parameter_classes.size())
			return nullptr;
		return s_parameter_classes[index];
	}

#define implement_parameter(name) trinex_implement_class(Engine::MaterialParameters::name, 0)

	PrimitiveBase& PrimitiveBase::update(const void* data, size_t size, const RHIShaderParameterInfo* info)
	{
		rhi->update_scalar_parameter(data, size, info->offset, info->binding);
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

	Float4x4& Float4x4::apply(const RendererContext& ctx, const RHIShaderParameterInfo* info)
	{
		update(&value, sizeof(Matrix4f), info);
		return *this;
	}

	LocalToWorld& LocalToWorld::apply(const RendererContext& ctx, const RHIShaderParameterInfo* info)
	{
		rhi->update_scalar_parameter(&ctx.local_to_world, sizeof(ctx.local_to_world), info->offset, info->binding);
		return *this;
	}

	Sampler::Sampler() : sampler(RHISamplerFilter::Point) {}

	Sampler& Sampler::apply(const RendererContext& ctx, const RHIShaderParameterInfo* info)
	{
		sampler.rhi_bind(info->binding);
		return *this;
	}

	bool Sampler::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		return ar.serialize(sampler);
	}

	Sampler2D::Sampler2D() : sampler(RHISamplerFilter::Point), texture(DefaultResources::Textures::default_texture) {}

	Sampler2D& Sampler2D::apply(const RendererContext& ctx, const RHIShaderParameterInfo* info)
	{
		auto rhi_sampler = sampler.rhi_sampler();

		if (texture && rhi_sampler)
		{
			rhi->bind_srv(texture->rhi_srv(), info->binding);
			rhi->bind_sampler(rhi_sampler, info->binding);
		}
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

	Texture2D& Texture2D::apply(const RendererContext& ctx, const RHIShaderParameterInfo* info)
	{
		if (texture)
		{
			rhi->bind_srv(texture->rhi_srv(), info->binding);
		}
		return *this;
	}

	bool Texture2D::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		ar.serialize_reference(texture);
		return true;
	}

	Globals& Globals::apply(const RendererContext& ctx, const RHIShaderParameterInfo* info)
	{
		auto buffer = ctx.renderer->globals_uniform_buffer();
		rhi->bind_uniform_buffer(buffer, info->binding);
		return *this;
	}

	Surface::Surface() : surface(nullptr) {}

	Surface& Surface::apply(const RendererContext& ctx, const RHIShaderParameterInfo* info)
	{
		auto srv = surface ? surface->rhi_srv() : DefaultResources::Textures::default_texture->rhi_srv();
		rhi->bind_srv(srv, info->binding);
		return *this;
	}

	bool Surface::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		return ar.serialize_reference(surface);
	}

	CombinedSurface::CombinedSurface() : surface(nullptr), sampler() {}

	CombinedSurface& CombinedSurface::apply(const RendererContext& ctx, const RHIShaderParameterInfo* info)
	{
		RHI_ShaderResourceView* srv = surface ? surface->rhi_srv() : nullptr;
		RHI_Sampler* rhi_sampler    = sampler.rhi_sampler();

		if (srv && rhi_sampler)
		{
			rhi->bind_srv(srv, info->binding);
			rhi->bind_sampler(rhi_sampler, info->binding);
		}
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
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Int)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(UInt)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Bool2)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Bool3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Bool4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Int2)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Int3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Int4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(UInt2)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(UInt3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(UInt4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float2)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float3x3)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(Float4x4)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, value, Refl::Property::IsTransient);
	}

	implement_parameter(LocalToWorld)
	{
		register_parameter<This>();
	}

	implement_parameter(Sampler)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, sampler, Refl::Property::IsTransient);
	}

	implement_parameter(Sampler2D)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, texture, Refl::Property::IsTransient);
		trinex_refl_prop(static_reflection(), This, sampler, Refl::Property::IsTransient);
	}

	implement_parameter(Texture2D)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, texture, Refl::Property::IsTransient);
	}

	implement_parameter(Globals)
	{
		register_parameter<This>();
	}

	implement_parameter(Surface)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, surface, Refl::Property::IsTransient);
	}

	implement_parameter(CombinedSurface)
	{
		register_parameter<This>();
		trinex_refl_prop(static_reflection(), This, surface, Refl::Property::IsTransient);
		trinex_refl_prop(static_reflection(), This, sampler, Refl::Property::IsTransient);
	}
}// namespace Engine::MaterialParameters
