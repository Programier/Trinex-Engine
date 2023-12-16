#pragma once

#include <Core/color_format.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/render_resource.hpp>
#include <Core/rhi_initializers.hpp>


namespace Engine
{
    class ENGINE_EXPORT Shader : public RenderResource
    {
    public:
        declare_class(Shader, RenderResource);

    public:
        struct UniformBuffer {
            Name name;
            size_t size;
            BindLocation location;
        };

        struct SSBO {
            Name name;
            BindLocation location;
        };

        struct Texture {
            Name name;
            BindLocation location;
        };

        using Sampler         = Texture;
        using SamplerCombined = Texture;

        Vector<UniformBuffer> uniform_buffers;
        Vector<Sampler> samplers;
        Vector<Texture> textures;
        Vector<Texture> combined_samplers;
        Vector<SSBO> ssbo;

        Buffer text_code;
        Buffer binary_code;
        BindLocation global_ubo_location;


        Shader& init_global_ubo(BindLocation location);
    };

    class ENGINE_EXPORT VertexShader : public Shader
    {
        declare_class(VertexShader, Shader);

    public:
        struct Attribute {
            String name;
            ColorFormat format;
            VertexAttributeInputRate rate;
            byte count = 1;
        };

        Vector<Attribute> attributes;

    public:
        VertexShader& rhi_create() override;
    };

    class ENGINE_EXPORT FragmentShader : public Shader
    {
        declare_class(FragmentShader, Shader);

    public:
        FragmentShader& rhi_create() override;
    };

}// namespace Engine
