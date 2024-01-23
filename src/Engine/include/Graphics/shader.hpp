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
        declare_class(Shader, RenderResource);

    private:
        bool load_source();
        bool save_source();

    public:
        enum class Type
        {
            Vertex,
            Fragment,
        };

        struct UniformBuffer {
            Name name;
            size_t size;
            BindLocation location;

            FORCE_INLINE UniformBuffer(size_t size = 0, BindLocation location = {}, Name name = Name())
                : name(name), size(size), location(location)
            {}
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

        String text_code;
        Buffer binary_code;

        bool archive_process(Archive& ar) override;
        virtual Type type() const = 0;
    };


    class ENGINE_EXPORT VertexShader : public Shader
    {
        declare_class(VertexShader, Shader);

    public:
        struct Attribute {
            Name name;
            ColorFormat format;
            VertexAttributeInputRate rate;
            byte count = 1;

            FORCE_INLINE Attribute(ColorFormat format            = ColorFormat::Undefined,
                                   VertexAttributeInputRate rate = VertexAttributeInputRate::Vertex, byte count = 1,
                                   const Name& name = Name::none)
                : name(name), format(format), rate(rate), count(count)
            {}
        };

        Vector<Attribute> attributes;

    public:
        VertexShader& rhi_create() override;
        bool archive_process(Archive& ar) override;
        Type type() const override;
    };

    class ENGINE_EXPORT FragmentShader : public Shader
    {
        declare_class(FragmentShader, Shader);

    public:
        FragmentShader& rhi_create() override;
        Type type() const override;
    };

    ENGINE_EXPORT bool operator&(Archive&, Shader::UniformBuffer&);
    ENGINE_EXPORT bool operator&(Archive&, Shader::SSBO&);
    ENGINE_EXPORT bool operator&(Archive&, Shader::Texture&);
    ENGINE_EXPORT bool operator&(Archive&, VertexShader::Attribute&);
}// namespace Engine
