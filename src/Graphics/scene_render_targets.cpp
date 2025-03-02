#include <Core/base_engine.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>
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

	RenderSurface* SceneRenderTargets::surface_of(Surface type) const
	{
		return m_surfaces.at(static_cast<size_t>(type)).ptr();
	}

	StringView SceneRenderTargets::name_of(Surface type) const
	{
		switch (type)
		{
			case Surface::SceneColorHDR:
				return "SceneColorHDR";
			case Surface::SceneColorLDR:
				return "SceneColorLDR";
			case Surface::SceneDepthZ:
				return "SceneDepthZ";
			case Surface::HitProxies:
				return "HitProxies";
			case Surface::BaseColor:
				return "BaseColor";
			case Surface::Normal:
				return "Normal";
			case Surface::Emissive:
				return "Emissive";
			case Surface::MSRA:
				return "MSRA";
			case Surface::LightPassDepthZ:
				return "LightPassDepthZ";

			default:
				throw EngineException("Undefined name of render target texture");
		}
	}

	ColorFormat SceneRenderTargets::format_of(Surface type) const
	{
		switch (type)
		{
			case Surface::SceneColorHDR:
				return ColorFormat::FloatRGBA;
			case Surface::SceneColorLDR:
				return ColorFormat::R8G8B8A8;
			case Surface::SceneDepthZ:
				return ColorFormat::DepthStencil;
			case Surface::HitProxies:
				return ColorFormat::R8G8B8A8;
			case Surface::BaseColor:
				return ColorFormat::R8G8B8A8;
			case Surface::Normal:
				return ColorFormat::FloatRGBA;
			case Surface::Emissive:
				return ColorFormat::R8G8B8A8;
			case Surface::MSRA:
				return ColorFormat::R8G8B8A8;
			case Surface::LightPassDepthZ:
				return ColorFormat::DepthStencil;

			default:
				throw EngineException("Undefined type of render target texture");
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

	const SceneRenderTargets& SceneRenderTargets::bind_scene_color_hdr(bool with_depth) const
	{

		rhi->bind_render_target1(surface_of(Surface::SceneColorHDR), with_depth ? surface_of(Surface::SceneDepthZ) : nullptr);
		return *this;
	}

	const SceneRenderTargets& SceneRenderTargets::bind_scene_color_ldr(bool with_depth) const
	{
		rhi->bind_render_target1(surface_of(Surface::SceneColorLDR), with_depth ? surface_of(Surface::SceneDepthZ) : nullptr);
		return *this;
	}

	const SceneRenderTargets& SceneRenderTargets::bind_gbuffer() const
	{
		rhi->bind_render_target(surface_of(Surface::BaseColor), surface_of(Surface::Normal), surface_of(Surface::Emissive),
								surface_of(Surface::MSRA), surface_of(Surface::SceneDepthZ));
		return *this;
	}

	const SceneRenderTargets& SceneRenderTargets::clear() const
	{
		surface_of(SceneColorHDR)->rhi_clear_color({0.f, 0.f, 0.f, 1.f});
		surface_of(SceneColorLDR)->rhi_clear_color({0.f, 0.f, 0.f, 1.f});
		surface_of(BaseColor)->rhi_clear_color({0.f, 0.f, 0.f, 1.f});
		surface_of(Normal)->rhi_clear_color({0.f, 0.f, 0.f, 1.f});
		surface_of(Emissive)->rhi_clear_color({0.f, 0.f, 0.f, 1.f});
		surface_of(MSRA)->rhi_clear_color({0.f, 0.f, 0.f, 1.f});
		surface_of(SceneDepthZ)->rhi_clear_depth_stencil(1.f, 0);
		surface_of(LightPassDepthZ)->rhi_clear_depth_stencil(1.f, 0);
		return *this;
	}
}// namespace Engine
