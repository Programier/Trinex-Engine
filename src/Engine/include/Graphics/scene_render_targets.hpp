#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/executable_object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/render_target.hpp>

namespace Engine
{
    class RenderTargetTexture;

    enum class SceneRenderTargetTexture
    {
        SceneColorHDR   = 0, /**< Render target for scene colors in HDR mode */
        SceneColorLDR   = 1, /**< Render target for scene colors in LDR mode */
        SceneDepthZ     = 2, /**< Render target for scene depths */
        HitProxies      = 3, /**< Render target for hit proxies */
        BaseColor       = 4, /**< Render target for base color */
        Normal          = 5, /**< Render target for normal */
        Emissive        = 6, /**< Render target for emissive */
        MSRA            = 7, /**< Render target for MSRA */
        LightPassDepthZ = 8, /**< Render target for light pass depths */
        __COUNT__       = 9,
    };


    class ENGINE_EXPORT EngineRenderTargets : public Singletone<EngineRenderTargets, EmptyClass>
    {
    public:
        static constexpr inline size_t textures_count = static_cast<size_t>(SceneRenderTargetTexture::__COUNT__);

    private:
        static EngineRenderTargets* m_instance;

        Array<Pointer<RenderTargetTexture>, textures_count> m_textures;
        UIntVector2D m_size;

    public:
        EngineRenderTargets();

        RenderTargetTexture* texture_of(SceneRenderTargetTexture type);
        ColorFormat format_of(SceneRenderTargetTexture type);
        StringView name_of(SceneRenderTargetTexture type);
        void initialize(UIntVector2D new_size);
        const UIntVector2D& size() const;
        uint_t width() const;
        uint_t height() const;

        friend class Singletone<EngineRenderTargets, EmptyClass>;
    };

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
        // [float3(color), opacity]
        RenderTargetTexture* base_color() const;
        // [float3(position), w]
        RenderTargetTexture* position() const;
        // [float3(normal), facing_sign);
        RenderTargetTexture* normal() const;
        // [float4(emissive));
        RenderTargetTexture* emissive() const;
        // [float4(metalic, specular, roughness, AO));
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
