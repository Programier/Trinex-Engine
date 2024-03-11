#pragma once

#include <Core/color_format.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/render_resource.hpp>
#include <Core/rhi_initializers.hpp>
#include <Graphics/shader_parameters.hpp>


namespace Engine
{
    class ENGINE_EXPORT Shader : public RenderResource
    {
        declare_class(Shader, RenderResource);

    public:
        enum class Type
        {
            Vertex,
            Fragment,
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

        Vector<Sampler> samplers;
        Vector<Texture> textures;
        Vector<SamplerCombined> combined_samplers;
        Vector<SSBO> ssbo;
        Buffer source_code;

        bool archive_process(Archive& ar) override;
        Shader& clean();
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
            VertexBufferSemantic semantic;
            byte semantic_index;
            byte count;

            FORCE_INLINE Attribute(ColorFormat format            = ColorFormat::Undefined,
                                   VertexAttributeInputRate rate = VertexAttributeInputRate::Vertex,
                                   VertexBufferSemantic semantic = VertexBufferSemantic::Position, byte semantic_index = 0,
                                   byte count = 1, const Name& name = Name::none)
                : name(name), format(format), rate(rate), semantic(semantic), semantic_index(semantic_index), count(count)
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

    ENGINE_EXPORT bool operator&(Archive&, Shader::SSBO&);
    ENGINE_EXPORT bool operator&(Archive&, Shader::Texture&);
    ENGINE_EXPORT bool operator&(Archive&, VertexShader::Attribute&);
}// namespace Engine
