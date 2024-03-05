#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/executable_object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/render_target.hpp>

namespace Engine
{
    class Texture2D;

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

    public:
        struct ENGINE_EXPORT Frame : public RenderTarget::Frame {
            Texture2D* base_color() const;
            Texture2D* position() const;
            Texture2D* normal() const;
            Texture2D* emissive() const;
            Texture2D* msra_buffer() const;
            Texture2D* depth() const;
        };

    private:
        GBuffer();
        ~GBuffer();

    public:
        Frame* current_frame() const;
        Frame* frame(byte index) const;
        friend class Singletone<GBuffer, EngineRenderTarget>;
        friend class Object;
    };

    class ENGINE_EXPORT GBufferBaseColorOutput : public Singletone<GBufferBaseColorOutput, EngineRenderTarget>
    {
        declare_class(GBufferBaseColorOutput, EngineRenderTarget);

    public:
        struct ENGINE_EXPORT Frame : public RenderTarget::Frame {
            Texture2D* texture() const;
        };

    private:
        GBufferBaseColorOutput();
        ~GBufferBaseColorOutput();

    public:
        Frame* current_frame() const;
        Frame* frame(byte index) const;

        friend class Singletone<GBufferBaseColorOutput, EngineRenderTarget>;
        friend class Object;
    };

    class ENGINE_EXPORT SceneColorOutput : public Singletone<SceneColorOutput, EngineRenderTarget>
    {
        declare_class(SceneColorOutput, EngineRenderTarget);

    public:
        struct ENGINE_EXPORT Frame : public RenderTarget::Frame {
            Texture2D* texture() const;
        };

    private:
        SceneColorOutput();
        ~SceneColorOutput();

    public:
        Frame* current_frame() const;
        Frame* frame(byte index) const;

        friend class Singletone<SceneColorOutput, EngineRenderTarget>;
        friend class Object;
    };

    void ENGINE_EXPORT update_render_targets_size();
}// namespace Engine
