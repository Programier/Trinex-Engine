#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture.hpp>


namespace Engine
{
    implement_engine_class_default_init(MaterialInterface);
    implement_engine_class_default_init(MaterialObject);

    implement_engine_class(Material, Class::IsAsset);
    implement_default_initialize_class(Material);
    implement_engine_class(MaterialInstance, Class::IsAsset);
    implement_default_initialize_class(MaterialInstance);

    Material::Material()
    {
        pipeline = Object::new_instance<Pipeline>();
    }
}// namespace Engine
