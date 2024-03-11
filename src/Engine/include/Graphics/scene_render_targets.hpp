#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/executable_object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/render_target.hpp>

namespace Engine
{
    class RenderTargetTexture;

    class ENGINE_EXPORT EngineRenderTarget : public RenderTarget
    {
        declare_class(EngineRenderTarget, RenderTarget);

    protected:
        bool m_enable_color_initialize         = true;
        bool m_enable_depth_stencil_initialize = true;

        void init(const Size2D& size, bool is_reinit = false);

    public:
        EngineRenderTarget& resize(const Size2D& new_size);
        bool is_engine_resource() const override;
    };

    class ENGINE_EXPORT GBuffer : public Singletone<GBuffer, EngineRenderTarget>
    {
        declare_class(GBuffer, EngineRenderTarget);

    private:
        GBuffer();
        ~GBuffer();

    public:
        RenderTargetTexture* base_color() const;
        RenderTargetTexture* position() const;
        RenderTargetTexture* normal() const;
        RenderTargetTexture* emissive() const;
        RenderTargetTexture* msra_buffer() const;
        RenderTargetTexture* depth() const;

        friend class Singletone<GBuffer, EngineRenderTarget>;
    };

    class ENGINE_EXPORT GBufferBaseColorOutput : public Singletone<GBufferBaseColorOutput, EngineRenderTarget>
    {
        declare_class(GBufferBaseColorOutput, EngineRenderTarget);

    private:
        GBufferBaseColorOutput();
        ~GBufferBaseColorOutput();

    public:
        RenderTargetTexture* texture() const;

        friend class Singletone<GBufferBaseColorOutput, EngineRenderTarget>;
    };

    class ENGINE_EXPORT SceneColorOutput : public Singletone<SceneColorOutput, EngineRenderTarget>
    {
        declare_class(SceneColorOutput, EngineRenderTarget);

    private:
        SceneColorOutput();
        ~SceneColorOutput();

    public:
        RenderTargetTexture* texture() const;
        friend class Singletone<SceneColorOutput, EngineRenderTarget>;
    };

    void ENGINE_EXPORT update_render_targets_size();
}// namespace Engine
