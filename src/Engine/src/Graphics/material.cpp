#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
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
    implement_initialize_class(Material)
    {
        Class* self = static_class_instance();

        self->add_properties(new ObjectProperty("Pipeline", "Pipeline settings for this material", &Material::pipeline));
    }

    implement_engine_class(MaterialInstance, Class::IsAsset);
    implement_default_initialize_class(MaterialInstance);


    MaterialInterface* MaterialInterface::parent()
    {
        return nullptr;
    }

    MaterialInterface& MaterialInterface::apply()
    {
        return *this;
    }

    Material::Material()
    {
        pipeline = Object::new_instance<Pipeline>();
    }

    bool Material::archive_process(Archive& archive)
    {
        return pipeline->archive_process(archive);
    }

    Material& Material::preload()
    {
        pipeline->preload();
        return *this;
    }

    Material& Material::postload()
    {
        pipeline->postload();
        return *this;
    }


    Material::~Material()
    {
        delete pipeline;
    }
}// namespace Engine
