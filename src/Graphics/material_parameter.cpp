#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/structures.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine::MaterialParameters
{
#define implement_parameter(name) implement_class(Engine::MaterialParameters, name, Class::IsAsset)


	PrimitiveBase& PrimitiveBase::update(const void* data, size_t size, MaterialParameterInfo* info)
	{
		rhi->update_local_parameter(data, size, info->offset);
		return *this;
	}

	bool PrimitiveBase::serialize(Archive& ar, void* data, size_t size)
	{
		if (ar.is_reading())
			ar.read_data(reinterpret_cast<byte*>(data), size);
		else
			ar.write_data(reinterpret_cast<const byte*>(data), size);
		return ar;
	}

	Float4x4& Float4x4::apply(SceneComponent* component, Pipeline* pipeline, MaterialParameterInfo* info)
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

	Sampler& Sampler::apply(SceneComponent* component, Pipeline* pipeline, MaterialParameterInfo* info)
	{
		if (sampler)
			sampler->rhi_bind(info->location);
		return *this;
	}

	bool Sampler::archive_process(Archive& ar)
	{
		if (!Super::archive_process(ar))
			return false;
		return sampler.archive_process(ar);
	}

	Sampler2D& Sampler2D::apply(SceneComponent* component, Pipeline* pipeline, MaterialParameterInfo* info)
	{
		if (sampler && texture)
			texture->rhi_bind_combined(sampler, info->location);
		return *this;
	}

	bool Sampler2D::archive_process(Archive& ar)
	{
		if (!Super::archive_process(ar))
			return false;

		sampler.archive_process(ar);
		texture.archive_process(ar);
		return ar;
	}

	Texture2D& Texture2D::apply(SceneComponent* component, Pipeline* pipeline, MaterialParameterInfo* info)
	{
		if (texture)
			texture->rhi_bind(info->location);
		return *this;
	}

	bool Texture2D::archive_process(Archive& ar)
	{
		if (!Super::archive_process(ar))
			return false;
		return texture.archive_process(ar);
	}

	implement_parameter(Parameter)
	{}

	implement_parameter(Bool)
	{}

	implement_parameter(Int)
	{}

	implement_parameter(UInt)
	{}

	implement_parameter(Float)
	{}

	implement_parameter(Bool2)
	{}

	implement_parameter(Bool3)
	{}

	implement_parameter(Bool4)
	{}

	implement_parameter(Int2)
	{}

	implement_parameter(Int3)
	{}

	implement_parameter(Int4)
	{}

	implement_parameter(UInt2)
	{}

	implement_parameter(UInt3)
	{}

	implement_parameter(UInt4)
	{}

	implement_parameter(Float2)
	{}

	implement_parameter(Float3)
	{}

	implement_parameter(Float4)
	{}

	implement_parameter(Float3x3)
	{}

	implement_parameter(Float4x4)
	{}

	implement_parameter(Sampler)
	{}

	implement_parameter(Sampler2D)
	{}

	implement_parameter(Texture2D)
	{}
}// namespace Engine::MaterialParameters
