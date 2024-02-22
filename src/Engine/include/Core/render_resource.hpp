#pragma once
#include <Core/engine_types.hpp>
#include <Core/executable_object.hpp>
#include <Core/object.hpp>
#include <Core/structures.hpp>

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


    class ENGINE_EXPORT RenderResource : public Object
    {
        declare_class(RenderResource, Object);

    protected:
        struct ENGINE_EXPORT DestroyRenderResource {
            void operator()(RHI_Object* object) const;
        };

        ScopedPtr<RHI_Object, DestroyRenderResource> m_rhi_object;


    public:
        RenderResource();

        bool has_object() const;
        template<typename T>
        T* rhi_object() const
        {
            return reinterpret_cast<T*>(m_rhi_object.get());
        }

        virtual RenderResource& rhi_create();
        RenderResource& init_resource(bool wait_initialize = false);
        RenderResource& rhi_destroy();

        RenderResource& postload() override;

        static void release_render_resouce(RHI_Object* object);
        ~RenderResource();
    };


    class ENGINE_EXPORT BindedRenderResource : public RenderResource
    {
        declare_class(BindedRenderResource, RenderResource);

    public:
        const BindedRenderResource& rhi_bind(BindLocation location) const;
    };

    struct ENGINE_EXPORT InitRenderResourceTask : public ExecutableObject {
    public:
        RenderResource* resource = nullptr;
        bool wait                = false;

        InitRenderResourceTask(RenderResource* object = nullptr, bool wait = false);
        int_t execute() override;
    };
}// namespace Engine
