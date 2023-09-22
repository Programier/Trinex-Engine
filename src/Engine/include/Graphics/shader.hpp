#pragma once

#include <Core/api_object.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/object_ref.hpp>
#include <Core/resource.hpp>
#include <Core/rhi_initializers.hpp>


namespace Engine
{
    struct PipelineCreateInfo;

    class ENGINE_EXPORT Shader : public ApiObject
    {
        declare_class(Shader, ApiObject);

    public:
        delete_copy_constructors(Shader);
        constructor_hpp(Shader);
        Shader(const PipelineCreateInfo& params);

        Shader& load(const PipelineCreateInfo& params);

        const Shader& use() const;
        static void unbind();
    };


    class ENGINE_EXPORT ShaderBase : public ApiObject
    {
    public:
        declare_class(ShaderBase, ApiObject);


    public:
        struct UniformBuffer {
            String name;
            size_t size;
            BindingIndex binding;
            BindingIndex set;
        };

        struct Texture {
            String name;
            BindingIndex binding;
            BindingIndex set;
        };

        using Sampler         = Texture;
        using SamplerCombined = Texture;

        Vector<ShaderUniformBuffer> uniform_buffers;
        Vector<Sampler> samplers;
        Vector<Texture> textures;
        Vector<Texture> combined_samplers;

        Buffer text_code;
        Buffer binary_code;

        ShaderBase& rhi_create() override;
    };

    class ENGINE_EXPORT VertexShader : public ShaderBase
    {
        declare_class(VertexShader, ShaderBase);

    public:
        struct Attribute {
            String name;
            ShaderDataType type;
            VertexAttributeInputRate rate;
        };

        Vector<Attribute> attributes;

    public:
        VertexShader& rhi_create() override;
    };

    class ENGINE_EXPORT FragmentShader : public ShaderBase
    {
        declare_class(FragmentShader, ShaderBase);

    public:
    };

}// namespace Engine
