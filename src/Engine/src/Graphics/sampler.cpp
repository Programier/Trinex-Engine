#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/sampler.hpp>
#include <api.hpp>


namespace Engine
{
    implement_class(Sampler, "Engine");
    implement_default_initialize_class(Sampler);

    Sampler& Sampler::create()
    {
        ResourceType* rs = resources();
        if (rs == nullptr)
        {
            error_log("Sampler", "Cannot create sampler, because resources is not found!");
        }
        destroy();
        _M_rhi_sampler = engine_instance->api_interface()->create_sampler(*rs);
        return *this;
    }

    bool Sampler::archive_process(Archive* archive)
    {
        if (!Super::archive_process(archive))
        {
            return false;
        }

        ResourceType* rs = resources(archive->is_reading());
        if (rs == nullptr)
            return false;

        (*archive) & (*rs);

        if (archive->is_reading())
        {
            create();
        }
        return *archive;
    }
}// namespace Engine
