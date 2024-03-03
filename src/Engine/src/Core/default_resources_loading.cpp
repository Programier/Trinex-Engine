#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/default_resources_loading.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>
#include <DefaultResources/default.hpp>

namespace Engine
{
    ENGINE_EXPORT Object*  load_object_from_memory(const byte* data, size_t size, const StringView& name, class Package* package)
    {
        if (size > 0)
        {
            Vector<byte> tmp(data, data + size);
            VectorReader reader = &tmp;
            if(Object* object = Object::load_object(&reader))
            {
                if(!name.empty())
                {
                    object->name(name);
                }

                if(package)
                {
                    package->add_object(object);
                }

                return object;
            }
        }

        return nullptr;
    }

    void EngineInstance::load_default_resources()
    {


        Package* package = Package::find_package("DefaultPackage", true);
        load_object_from_memory(DefaultSampler_data, DefaultSampler_len, "DefaultSampler", package);
        load_object_from_memory(DefaultTexture_data, DefaultTexture_len, "DefaultTexture", package);
        load_object_from_memory(SpriteMaterial_data, SpriteMaterial_len, "SpriteMaterial", package);
        load_object_from_memory(ScreenPositionBuffer_data, ScreenPositionBuffer_len, "ScreenPositionBuffer", package);

        load_object_from_memory(BaseColorToScreenMat_data, BaseColorToScreenMat_len, "BaseColorToScreenMat", package);
        load_object_from_memory(DefaultMaterial_data, DefaultMaterial_len, "DefaultMaterial", package);
        load_object_from_memory(GBufferLinesMat_data, GBufferLinesMat_len, "GBufferLinesMat", package);
        load_object_from_memory(SceneOutputLinesMat_data, SceneOutputLinesMat_len, "SceneOutputLinesMat", package);

        DefaultResourcesInitializeController().execute();
        m_flags(DefaultResourcesInitTriggered, true);
    }
}// namespace Engine
