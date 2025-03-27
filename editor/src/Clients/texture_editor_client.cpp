#include <Clients/texture_editor_client.hpp>
#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/editor_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <Widgets/property_renderer.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	static inline void copy_texture_to_surface(RenderSurface* dst, RHI_ShaderResourceView* srv, float power, uint_t level,
											   Swizzle swizzle)
	{
		Rect2D rect;
		rect.pos  = {0, 0};
		rect.size = dst->size();

		auto pipeline = Pipelines::Blit2DGamma::instance();
		pipeline->blit(srv, dst->rhi_unordered_access_view(), rect, rect, power, level, swizzle);
	}

	static inline Swizzle modify_swizzle(Swizzle swizzle, ColorFormat format)
	{
		if (format.is_depth())
		{
			static const Swizzle::Enum depth_swizzle[] = {
					Swizzle::R,
					Swizzle::R,
					Swizzle::R,
					Swizzle::One,
			};

			byte* values = &swizzle.r;
			for (uint_t i = 0; i < 4; ++i)
			{
				if (values[i] != Swizzle::Zero)
					values[i] = depth_swizzle[i];
			}
		}
		return swizzle;
	}

	static Vector2f max_texture_size_in_viewport(const Vector2f& texture_size, const Vector2f& viewport_size)
	{
		float texture_aspect_ratio  = texture_size.x / texture_size.y;
		float viewport_aspect_ratio = viewport_size.x / viewport_size.y;

		Vector2f result_size;

		if (texture_aspect_ratio > viewport_aspect_ratio)
		{
			result_size.x = viewport_size.x;
			result_size.y = viewport_size.x / texture_aspect_ratio;
		}
		else
		{
			result_size.y = viewport_size.y;
			result_size.x = viewport_size.y * texture_aspect_ratio;
		}

		return result_size;
	}

	trinex_implement_engine_class_default_init(TextureEditorClient, 0);

	trinex_implement_engine_class(Texture2DEditorClient, 0)
	{
		register_client(Texture::static_class_instance(), static_class_instance());
	}

	trinex_implement_engine_class(RenderSurfaceEditorClient, 0)
	{
		register_client(RenderSurface::static_class_instance(), static_class_instance());
	}

	TextureEditorClient::TextureEditorClient()
	{
		m_surface = Object::new_instance<RenderSurface>();
		m_surface->init(ColorFormat::R8G8B8A8, {1, 1});

		call_in_render_thread([self = Pointer(this)]() { self->m_surface->rhi_render_target_view()->clear(Color(0, 0, 0, 1)); });

		menu_bar.create("")->actions.push([this]() {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			static ImU32 colors[4] = {IM_COL32(255, 0, 0, 255), IM_COL32(0, 255, 0, 255), IM_COL32(0, 0, 255, 255),
									  IM_COL32(255, 255, 255, 255)};

			float height                                = ImGui::GetContentRegionAvail().y;
			static const char* channel_names[]          = {"###red", "###green", "###blue", "###alpha"};
			static const Swizzle::Enum swizzle_values[] = {Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::A};

			for (int i = 0; const char* name : channel_names)
			{
				byte* swizzle = &m_swizzle.x;
				if (ImGui::Selectable(name, swizzle[i] != Swizzle::Zero, 0, {height, height}))
				{
					swizzle[i] = swizzle[i] == Swizzle::Zero ? swizzle_values[i] : Swizzle::Zero;
					request_render();
				}

				auto min = ImGui::GetItemRectMin();
				auto max = ImGui::GetItemRectMax();

				auto center = (max + min) * 0.5f;

				if (swizzle[i] == Swizzle::Zero)
					draw_list->AddCircle(center, height / 2.f, colors[i], 0, 2.f);
				else
					draw_list->AddCircleFilled(center, height / 2.f, colors[i]);

				++i;
			}

			ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);

			if (ImGui::InputFloat("Pow", &m_pow))
			{
				m_pow = glm::clamp(m_pow, 0.001f, 100.f);
				request_render();
			}
		});
	}

	TextureEditorClient& TextureEditorClient::request_render()
	{
		size_t index = engine_instance->frame_index();

		if (m_last_render_frame_index != index)
		{
			m_last_render_frame_index = index;
			setup_surface(m_surface);
			render_thread()->call([self = Pointer(this)]() { self->rhi_render_surface(self->m_surface); });
		}
		return *this;
	}

	TextureEditorClient& TextureEditorClient::on_bind_viewport(RenderViewport* vp)
	{
		Super::on_bind_viewport(vp);
		auto current = ImGuiWindow::current();
		ImGuiWindow::make_current(imgui_window());
		m_properties           = imgui_window()->widgets_list.create<PropertyRenderer>();
		m_properties->closable = false;

		ImGuiWindow::make_current(current);
		return *this;
	}

	TextureEditorClient& TextureEditorClient::update(float dt)
	{
		Super::update(dt);

		ImGui::Begin("Texture View###texture");
		if (m_surface->rhi_texture())
		{
			ImGui::BeginHorizontal(0, ImGui::GetContentRegionAvail());
			ImGui::Spring(1.f, 0.5);
			auto size = max_texture_size_in_viewport(m_surface->size(), ImGui::EngineVecFrom(ImGui::GetContentRegionAvail()));
			ImGui::Image(ImTextureID(m_surface, DefaultResources::Samplers::default_sampler), ImGui::ImVecFrom(size));
			ImGui::Spring(1.f, 0.5);
			ImGui::EndHorizontal();
		}
		ImGui::End();

		return *this;
	}

	TextureEditorClient& TextureEditorClient::select(Object* object)
	{
		m_properties->update(object);
		return *this;
	}

	TextureEditorClient& TextureEditorClient::build_dock(uint32_t dock_id)
	{
		auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);
		ImGui::DockBuilderDockWindow("Texture View###texture", dock_id);
		ImGui::DockBuilderDockWindow(PropertyRenderer::static_name(), dock_id_right);
		return *this;
	}

	// TEXTURE 2D RENDERING

	Texture2DEditorClient& Texture2DEditorClient::setup_surface(RenderSurface* dst)
	{
		Vector2u size = m_texture->size(m_mip_index);

		if (size != dst->size())
		{
			dst->init(ColorFormat::R8G8B8A8, size);
		}
		return *this;
	}

	Texture2DEditorClient& Texture2DEditorClient::rhi_render_surface(RenderSurface* surface)
	{
		if (!m_texture)
			return *this;

		copy_texture_to_surface(surface, m_texture->rhi_shader_resource_view(), pow_factor(), m_mip_index,
								modify_swizzle(swizzle(), m_texture->format));
		return *this;
	}

	Texture2DEditorClient& Texture2DEditorClient::select(Object* object)
	{
		if (m_texture == object)
			return *this;

		if (Texture2D* texture = instance_cast<Texture2D>(object))
		{
			m_texture = texture;
			Super::select(object);

			request_render();
		}

		return *this;
	}

	// RENDER SURFACE RENDERING

	RenderSurfaceEditorClient::RenderSurfaceEditorClient()
	{
		menu_bar.create("")->actions.push([this]() { ImGui::Checkbox("Live Update", &m_live_update); });
	}

	RenderSurfaceEditorClient& RenderSurfaceEditorClient::setup_surface(RenderSurface* dst)
	{
		Vector2u size = m_surface->size();

		if (size != dst->size())
		{
			dst->init(ColorFormat::R8G8B8A8, size);
		}
		return *this;
	}

	RenderSurfaceEditorClient& RenderSurfaceEditorClient::rhi_render_surface(RenderSurface* surface)
	{
		if (!m_surface)
			return *this;

		copy_texture_to_surface(surface, m_surface->rhi_shader_resource_view(), pow_factor(), 0,
								modify_swizzle(swizzle(), m_surface->format()));
		return *this;
	}

	RenderSurfaceEditorClient& RenderSurfaceEditorClient::select(Object* object)
	{
		if (m_surface == object)
			return *this;

		if (RenderSurface* surface = instance_cast<RenderSurface>(object))
		{
			m_surface = surface;
			Super::select(object);
			request_render();
		}

		return *this;
	}

	RenderSurfaceEditorClient& RenderSurfaceEditorClient::update(float dt)
	{
		Super::update(dt);
		if (m_live_update)
			request_render();
		return *this;
	}
}// namespace Engine
