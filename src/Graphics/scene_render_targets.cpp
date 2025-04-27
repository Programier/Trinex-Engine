#include <Core/base_engine.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	SceneRenderTargets* SceneRenderTargets::s_instance = nullptr;

	SceneRenderTargets::SceneRenderTargets()
	{
		for (size_t texture_index = 0; texture_index < textures_count; ++texture_index)
		{
			Surface type              = static_cast<Surface>(texture_index);
			m_surfaces[texture_index] = Object::new_instance<RenderSurface>(
			        name_of(type), Object::static_find_package("TrinexEngine::RenderTargets", true));
		}
	}

	const Array<Pointer<RenderSurface>, SceneRenderTargets::textures_count>& SceneRenderTargets::surfaces() const
	{
		return m_surfaces;
	}

	inline RenderSurface* SceneRenderTargets::surface_of(Surface type) const
	{
		return m_surfaces.at(static_cast<size_t>(type)).ptr();
	}

	inline RHI_RenderTargetView* SceneRenderTargets::rtv_of(Surface type) const
	{
		return surface_of(type)->rhi_render_target_view();
	}

	inline RHI_DepthStencilView* SceneRenderTargets::dsv_of(Surface type) const
	{
		return surface_of(type)->rhi_depth_stencil_view();
	}

	StringView SceneRenderTargets::name_of(Surface type) const
	{
		switch (type)
		{
			case Surface::SceneColor: return "SceneColor";
			case Surface::SceneDepth: return "SceneDepth";
			case Surface::BaseColor: return "BaseColor";
			case Surface::Normal: return "Normal";
			case Surface::Emissive: return "Emissive";
			case Surface::MSRA: return "MSRA";
			case Surface::LightPassDepthZ: return "LightPassDepthZ";

			default: throw EngineException("Undefined name of render target texture");
		}
	}

	SurfaceFormat SceneRenderTargets::format_of(Surface type) const
	{
		switch (type)
		{
			case Surface::SceneColor: return Settings::Rendering::enable_hdr ? SurfaceFormat::RGBA16F : SurfaceFormat::RGBA8;
			case Surface::SceneDepth: return SurfaceFormat::Depth;
			case Surface::BaseColor: return SurfaceFormat::RGBA8;
			case Surface::Normal: return SurfaceFormat::RGBA16F;
			case Surface::Emissive: return SurfaceFormat::RGBA8;
			case Surface::MSRA: return SurfaceFormat::RGBA8;
			case Surface::LightPassDepthZ: return SurfaceFormat::ShadowDepth;

			default: throw EngineException("Undefined type of render target texture");
		}
	}

	void SceneRenderTargets::initialize(Size2D new_size)
	{
		if (new_size == m_size || new_size.x == 0 || new_size.y == 0 || (m_size.x >= new_size.x && m_size.y >= new_size.y))
			return;

		m_size = new_size;

		for (size_t texture_index = 0; texture_index < textures_count; ++texture_index)
		{
			m_surfaces[texture_index]->init(format_of(static_cast<Surface>(texture_index)), new_size);
		}
	}

	const Size2D& SceneRenderTargets::size() const
	{
		return m_size;
	}

	float SceneRenderTargets::width() const
	{
		return m_size.x;
	}

	float SceneRenderTargets::height() const
	{
		return m_size.y;
	}

	const SceneRenderTargets& SceneRenderTargets::bind_scene_color(bool with_depth) const
	{
		rhi->bind_render_target1(rtv_of(Surface::SceneColor), with_depth ? dsv_of(Surface::SceneDepth) : nullptr);
		return *this;
	}

	const SceneRenderTargets& SceneRenderTargets::bind_gbuffer() const
	{
		rhi->bind_render_target(rtv_of(Surface::BaseColor), rtv_of(Surface::Normal), rtv_of(Surface::Emissive),
		                        rtv_of(Surface::MSRA), dsv_of(Surface::SceneDepth));
		return *this;
	}

	const SceneRenderTargets& SceneRenderTargets::clear() const
	{
		LinearColor color = {0.f, 0.f, 0.f, 1.f};

		surface_of(SceneColor)->rhi_render_target_view()->clear(color);
		surface_of(BaseColor)->rhi_render_target_view()->clear(color);
		surface_of(Normal)->rhi_render_target_view()->clear(color);
		surface_of(Emissive)->rhi_render_target_view()->clear(color);
		surface_of(MSRA)->rhi_render_target_view()->clear(color);
		surface_of(SceneDepth)->rhi_depth_stencil_view()->clear(1.0, 0);
		surface_of(LightPassDepthZ)->rhi_depth_stencil_view()->clear(1.0, 0);
		return *this;
	}
}// namespace Engine
