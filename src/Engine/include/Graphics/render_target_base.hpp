#pragma once
#include <Core/pointer.hpp>
#include <Core/render_resource.hpp>
#include <Core/structures.hpp>


namespace Engine
{
    class RenderPass;

    class ENGINE_EXPORT RenderTargetBase : public RenderResource
    {
        declare_class(RenderTargetBase, RenderResource);

    public:
        struct GlobalUniforms {
            Matrix4f projview;
            Vector2D size;
            float time;
            float dt;
            float min_depth = 0.0f;
            float max_depth = 1.0f;
        };

        GlobalUniforms global_ubo;

    protected:
        static RenderTargetBase* _M_current;

        ViewPort _M_viewport;
        Scissor _M_scissor;
        Pointer<class UniformBuffer> _M_uniform_buffer;
        byte _M_frame_index = -1;

    public:
        Pointer<RenderPass> render_pass;

        delete_copy_constructors(RenderTargetBase);
        RenderTargetBase();

        RenderTargetBase& rhi_bind();
        const RenderTargetBase& viewport(const ViewPort& viewport);
        const RenderTargetBase& scissor(const Scissor& scissor);
        const ViewPort& viewport();
        const Scissor& scissor();
        const RenderTargetBase& clear_color(const ColorClearValue& color, byte layout = 0) const;
        const RenderTargetBase& clear_depth_stencil(const DepthStencilClearValue& value) const;
        RenderTargetBase& rhi_create() override;
        UniformBuffer* uniform_buffer() const;

        static RenderTargetBase* current_target();
        static void reset_current_target();

        ~RenderTargetBase();
    };
}// namespace Engine
