#pragma once

#include <Core/api_object.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/object_ref.hpp>
#include <Core/resource.hpp>
#include <Core/rhi_initializers.hpp>


namespace Engine
{
    class ENGINE_EXPORT Shader : public ApiObject
    {
    public:
        declare_class(Shader, ApiObject);


    public:
        struct UniformBuffer {
            String name;
            size_t size;
            BindingIndex binding;
            BindingIndex set;
        };

        struct SSBO {
            String name;
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

        Vector<UniformBuffer> uniform_buffers;
        Vector<Sampler> samplers;
        Vector<Texture> textures;
        Vector<Texture> combined_samplers;
        Vector<SSBO> ssbo;

        Buffer text_code;
        Buffer binary_code;
    };

    class ENGINE_EXPORT VertexShader : public Shader
    {
        declare_class(VertexShader, Shader);

    public:
        struct Attribute {
            String name;
            ShaderDataType type;
            VertexAttributeInputRate rate;
        };

        Vector<Attribute> attributes;

    public:
        VertexShader& rhi_create();
    };

    class ENGINE_EXPORT FragmentShader : public Shader
    {
        declare_class(FragmentShader, Shader);

    public:
        FragmentShader& rhi_create();
    };

}// namespace Engine
