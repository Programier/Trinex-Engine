#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>
#include <Graphics/types/color_format.hpp>

namespace Engine
{
	class RenderSurface;
	struct RHI_RenderTargetView;
	struct RHI_DepthStencilView;

	class ENGINE_EXPORT SceneRenderTargets : public Singletone<SceneRenderTargets, EmptyClass>
	{
	public:
		enum Surface
		{
			SceneColor      = 0, /**< Render target for scene colors */
			SceneDepth      = 1, /**< Render target for scene depths */
			BaseColor       = 2, /**< Render target for base color */
			Normal          = 3, /**< Render target for normal */
			Emissive        = 4, /**< Render target for emissive */
			MSRA            = 5, /**< Render target for MSRA */
			LightPassDepthZ = 6, /**< Render target for light pass depths */
			__COUNT__       = 7,
		};

		static constexpr inline size_t textures_count = static_cast<size_t>(Surface::__COUNT__);

	private:
		static SceneRenderTargets* s_instance;

		Array<Pointer<RenderSurface>, textures_count> m_surfaces;
		Size2D m_size;

	public:
		SceneRenderTargets();

		const Array<Pointer<RenderSurface>, textures_count>& surfaces() const;
		RenderSurface* surface_of(Surface type) const;
		RHI_RenderTargetView* rtv_of(Surface type) const;
		RHI_DepthStencilView* dsv_of(Surface type) const;
		SurfaceFormat format_of(Surface type) const;
		StringView name_of(Surface type) const;
		void initialize(Size2D new_size);
		const Size2D& size() const;
		float width() const;
		float height() const;

		const SceneRenderTargets& bind_scene_color(bool with_depth = true) const;
		const SceneRenderTargets& bind_gbuffer() const;
		const SceneRenderTargets& clear() const;

		friend class Singletone<SceneRenderTargets, EmptyClass>;
	};
}// namespace Engine
