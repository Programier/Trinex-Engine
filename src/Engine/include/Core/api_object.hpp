#pragma once
#include <Core/engine_types.hpp>
#include <Core/object.hpp>

namespace Engine
{
    struct RHI_Object;
    struct RHI_BindingObject;
    struct RHI_Sampler;
    struct RHI_RenderTarget;
    struct RHI_Texture;
    struct RHI_Shader;
    struct RHI_Pipeline;
    struct RHI_Buffer;
    struct RHI_VertexBuffer;
    struct RHI_IndexBuffer;
    struct RHI_UniformBuffer;
    struct RHI_SSBO;
    struct RHI_RenderPass;


    class ENGINE_EXPORT ApiObject : public Object
    {
        declare_class(ApiObject, Object);

    protected:
        union
        {
            RHI_Object* _M_rhi_object;
            RHI_BindingObject* _M_rhi_binding_object;
            RHI_Sampler* _M_rhi_sampler;
            RHI_RenderTarget* _M_rhi_render_target;
            RHI_Texture* _M_rhi_texture;
            RHI_Shader* _M_rhi_shader;
            RHI_Pipeline* _M_rhi_pipeline;
            RHI_Buffer* _M_rhi_buffer;
            RHI_VertexBuffer* _M_rhi_vertex_buffer;
            RHI_IndexBuffer* _M_rhi_index_buffer;
            RHI_UniformBuffer* _M_rhi_uniform_buffer;
            RHI_SSBO* _M_rhi_ssbo;
            RHI_RenderPass* _M_rhi_render_pass;
        };

        bool _M_can_delete;

    public:
        ApiObject();

        bool has_object() const;
        template<typename T>
        T* rhi_object() const
        {
            return reinterpret_cast<T*>(_M_rhi_object);
        }

        ApiObject& rhi_destroy();

        ~ApiObject();
    };


    class ApiBindingObject : public ApiObject
    {
        declare_class(ApiBindingObject, ApiObject);

    public:
        const ApiBindingObject& rhi_bind(BindingIndex binding, BindingIndex set = 0) const;
    };


}// namespace Engine
