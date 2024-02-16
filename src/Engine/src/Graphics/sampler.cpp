#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/enum.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>

namespace Engine
{
    implement_class(Sampler, Engine, Class::IsAsset);
    implement_initialize_class(Sampler)
    {
        Class* self     = static_class_instance();
        Enum* wrap_enum = Enum::find("Engine::WrapValue");

        self->add_properties(new BoolProperty("Unnormalized Coordinates", "True if sampler used unnormalized coordinates",
                                              &This::unnormalized_coordinates),

                             new FloatProperty("Mip lod bias", "Mip lod bias of sampler", &This::mip_lod_bias),
                             new FloatProperty("Anisotropy", "Anisotropy of sampler", &This::anisotropy),
                             new FloatProperty("Min lod", "Min lod of sampler", &This::min_lod),
                             new FloatProperty("Max lod", "Max lod of sampler", &This::max_lod),

                             new EnumProperty("Filter", "Filter of sampler", &This::filter, Enum::find("Engine::SamplerFilter")),
                             new EnumProperty("Wrap S", "Wrap S of sampler", &This::wrap_s, wrap_enum),
                             new EnumProperty("Wrap T", "Wrap T of sampler", &This::wrap_t, wrap_enum),
                             new EnumProperty("Wrap R", "Wrap R of sampler", &This::wrap_r, wrap_enum),
                             new EnumProperty("Compare Mode", "Compare Mode of sampler", &This::compare_mode,
                                              Enum::find("Engine::CompareMode")),
                             new EnumProperty("Compare Func", "Compare Func of sampler", &This::compare_func,
                                              Enum::find("Engine::CompareFunc")));
    }

    Sampler& Sampler::rhi_create()
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_sampler(this));
        return *this;
    }

    bool Sampler::archive_process(Archive& archive)
    {
        if (!Super::archive_process(archive))
        {
            return false;
        }

        archive & filter;
        archive & wrap_s;
        archive & wrap_t;
        archive & wrap_r;
        archive & mip_lod_bias;
        archive & anisotropy;
        archive & compare_mode;
        archive & min_lod;
        archive & max_lod;
        archive & compare_func;
        archive & unnormalized_coordinates;

        return archive;
    }

    Sampler& Sampler::reload()
    {
        init_resource();
        return *this;
    }
}// namespace Engine
