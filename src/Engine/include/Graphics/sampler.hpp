#pragma once
#include <Core/render_resource.hpp>
#include <Core/rhi_initializers.hpp>

namespace Engine
{


    class ENGINE_EXPORT Sampler : public BindedRenderResource
    {
        declare_class(Sampler, BindedRenderResource);

    public:
        SamplerFilter filter     = SamplerFilter::Point;
        WrapValue wrap_s         = WrapValue::Repeat;
        WrapValue wrap_t         = WrapValue::Repeat;
        WrapValue wrap_r         = WrapValue::Repeat;
        float mip_lod_bias       = 0.0;
        float anisotropy         = 1.0;
        CompareMode compare_mode = CompareMode::None;
        float min_lod            = -1000.0;
        float max_lod            = 1000.0;
        CompareFunc compare_func = CompareFunc::Always;
        bool unnormalized_coordinates;

        Sampler& rhi_create() override;
        bool archive_process(Archive& archive) override;
        Sampler& reload() override;
    };
}// namespace Engine
