#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>


namespace Engine
{
    implement_class(Sampler, "Engine", 0);
    implement_default_initialize_class(Sampler);

    Sampler& Sampler::rhi_create()
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_sampler(this));
        return *this;
    }

    bool Sampler::archive_process(Archive* archive)
    {
        if (!Super::archive_process(archive))
        {
            return false;
        }

        (*archive) & filter;
        (*archive) & wrap_s;
        (*archive) & wrap_t;
        (*archive) & wrap_r;
        (*archive) & mip_lod_bias;
        (*archive) & anisotropy;
        (*archive) & compare_mode;
        (*archive) & min_lod;
        (*archive) & max_lod;
        (*archive) & compare_func;
        (*archive) & unnormalized_coordinates;

        return *archive;
    }
}// namespace Engine
