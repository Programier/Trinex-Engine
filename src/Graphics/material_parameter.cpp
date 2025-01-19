#include <Core/archive.hpp>
#include <Core/default_resources.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/structures.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine::MaterialParameters
{
#define implement_parameter(name) implement_class(Engine::MaterialParameters::name, 0)

	PrimitiveBase& PrimitiveBase::update(const void* data, size_t size, MaterialParameterInfo* info)
	{
		rhi->update_scalar_parameter(data, size, info->offset);
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

	Float4x4& Float4x4::apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass, MaterialParameterInfo* info)
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

	Model4x4& Model4x4::apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass, MaterialParameterInfo* info)
	{
		auto matrix = component->proxy()->world_transform().matrix();
		rhi->update_scalar_parameter(&matrix, sizeof(matrix), info->offset);
		return *this;
	}

	Sampler::Sampler() : sampler(DefaultResources::Samplers::default_sampler)
	{}

	Sampler& Sampler::apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass, MaterialParameterInfo* info)
	{
		if (sampler)
			sampler->rhi_bind(info->location);
		return *this;
	}

	bool Sampler::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		return ar.serialize_reference(sampler);
	}

	Sampler2D::Sampler2D()
	    : sampler(DefaultResources::Samplers::default_sampler), texture(DefaultResources::Textures::default_texture)
	{}

	Sampler2D& Sampler2D::apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
								MaterialParameterInfo* info)
	{
		if (sampler && texture)
			texture->rhi_bind_combined(sampler, info->location);
		return *this;
	}

	bool Sampler2D::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;

		ar.serialize_reference(sampler);
		ar.serialize_reference(texture);
		return ar;
	}

	Texture2D::Texture2D() : texture(DefaultResources::Textures::default_texture)
	{}

	Texture2D& Texture2D::apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
								MaterialParameterInfo* info)
	{
		if (texture)
			texture->rhi_bind(info->location);
		return *this;
	}

	bool Texture2D::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		ar.serialize_reference(texture);
		return true;
	}

	implement_parameter(Parameter)
	{}

	implement_parameter(Bool)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Int)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(UInt)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Float)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Bool2)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Bool3)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Bool4)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Int2)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Int3)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Int4)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(UInt2)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(UInt3)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(UInt4)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Float2)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Float3)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Float4)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Float3x3)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Float4x4)
	{
		trinex_refl_prop(static_class_instance(), This, value);
	}

	implement_parameter(Model4x4)
	{}

	implement_parameter(Sampler)
	{
		trinex_refl_prop(static_class_instance(), This, sampler);
	}

	implement_parameter(Sampler2D)
	{
		trinex_refl_prop(static_class_instance(), This, texture);
		trinex_refl_prop(static_class_instance(), This, sampler);
	}

	implement_parameter(Texture2D)
	{
		trinex_refl_prop(static_class_instance(), This, texture);
	}
}// namespace Engine::MaterialParameters
