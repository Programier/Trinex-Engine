#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/executable_object.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>

namespace Engine
{
    class RenderSurface;


    class ENGINE_EXPORT SceneRenderTargets : public Singletone<SceneRenderTargets, EmptyClass>
    {
    public:
        enum Surface
        {
            SceneColorHDR   = 0, /**< Render target for scene colors in HDR mode */
            SceneColorLDR   = 1, /**< Render target for scene colors in LDR mode */
            SceneDepthZ     = 2, /**< Render target for scene depths */
            HitProxies      = 3, /**< Render target for hit proxies */
            BaseColor       = 4, /**< Render target for base color */
            Position        = 5, /**< Render target for position */
            Normal          = 6, /**< Render target for normal */
            Emissive        = 7, /**< Render target for emissive */
            MSRA            = 8, /**< Render target for MSRA */
            LightPassDepthZ = 9, /**< Render target for light pass depths */
            __COUNT__       = 10,
        };

        static constexpr inline size_t textures_count = static_cast<size_t>(Surface::__COUNT__);

    private:
        static SceneRenderTargets* m_instance;

        Array<Pointer<RenderSurface>, textures_count> m_surfaces;
        Size2D m_size;

    public:
        SceneRenderTargets();

        const Array<Pointer<RenderSurface>, textures_count>& surfaces() const;
        RenderSurface* surface_of(Surface type) const;
        ColorFormat format_of(Surface type) const;
        StringView name_of(Surface type) const;
        void initialize(Size2D new_size);
        const Size2D& size() const;
        float width() const;
        float height() const;

        const SceneRenderTargets& begin_rendering_scene_color_hdr() const;
        const SceneRenderTargets& end_rendering_scene_color_hdr() const;

        const SceneRenderTargets& begin_rendering_scene_color_ldr() const;
        const SceneRenderTargets& end_rendering_scene_color_ldr() const;

        const SceneRenderTargets& begin_rendering_gbuffer() const;
        const SceneRenderTargets& end_rendering_gbuffer() const;

        const SceneRenderTargets& clear() const;

        friend class Singletone<SceneRenderTargets, EmptyClass>;
    };
}// namespace Engine
