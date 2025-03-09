#include <Clients/texture_editor_client.hpp>
#include <Core/default_resources.hpp>
#include <Core/editor_resources.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_2D.hpp>
#include <Widgets/property_renderer.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	trinex_implement_engine_class(TextureEditorClient, 0)
	{
		register_client(Texture::static_class_instance(), static_class_instance());
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

	TextureEditorClient::TextureEditorClient()
	{
		m_surface = Object::new_instance<RenderSurface>();
		m_surface->init(ColorFormat::R8G8B8A8, {1, 1});

		call_in_render_thread([self = Pointer(this)]() { self->m_surface->rhi_render_target_view()->clear(Color(0, 0, 0, 1)); });

		menu_bar.create("")->actions.push([this]() {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			static ImU32 colors[4] = {IM_COL32(255, 0, 0, 255), IM_COL32(0, 255, 0, 255), IM_COL32(0, 0, 255, 255),
									  IM_COL32(255, 255, 255, 255)};

			float height                       = ImGui::GetContentRegionAvail().y;
			static const char* channel_names[] = {"###red", "###green", "###blue", "###alpha"};

			for (int i = 0; const char* name : channel_names)
			{
				if (ImGui::Selectable(name, m_channels_status[i], 0, {height, height}))
				{
					m_channels_status[i] = !m_channels_status[i];
					on_object_parameters_changed();
				}

				auto min = ImGui::GetItemRectMin();
				auto max = ImGui::GetItemRectMax();

				auto center = (max + min) * 0.5f;

				if (m_channels_status[i])
					draw_list->AddCircleFilled(center, height / 2.f, colors[i]);
				else
					draw_list->AddCircle(center, height / 2.f, colors[i], 0, 2.f);

				++i;
			}

			ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
			if (ImGui::InputFloat("Pow", &m_pow))
			{
				m_pow = glm::clamp(m_pow, 0.001f, 100.f);
				on_object_parameters_changed();
			}

			ImGui::Checkbox("Live Update", &m_live_update);
		});
	}

	TextureEditorClient& TextureEditorClient::render_texture()
	{
		ImGui::Begin("Texture View###texture");
		if (m_surface->rhi_surface())
		{
			ImGui::BeginHorizontal(0, ImGui::GetContentRegionAvail());
			ImGui::Spring(1.f, 0.5);
			auto size = max_texture_size_in_viewport(glm::max(m_texture->size(), {1, 1}),
													 ImGui::EngineVecFrom(ImGui::GetContentRegionAvail()));
			ImGui::Image(ImTextureID(m_surface, EditorResources::default_sampler), ImGui::ImVecFrom(size));
			ImGui::Spring(1.f, 0.5);
			ImGui::EndHorizontal();
		}
		ImGui::End();

		return *this;
	}

	static void render_texture_to_surface(RenderSurface* surface, Texture2D* texture, uint_t mip, Vector4f mask, float pow = 1.f)
	{
		ViewPort vp;
		vp.size = texture->size(0);
		vp.pos  = {0, 0};
		rhi->viewport(vp);

		Scissor scissor;
		scissor.size = texture->size(0);
		scissor.pos  = {0, 0};
		rhi->scissor(scissor);

		surface->rhi_render_target_view()->clear(Color(0.f, 0.f, 0.f, 0.f));
		rhi->bind_render_target1(surface->rhi_render_target_view());

		static Name mip_level_name = "mip_level";
		static Name power          = "power";
		auto mat                   = EditorResources::texture_editor_material;

		auto p_mask      = mat->find_parameter<MaterialParameters::Float4>(Name::mask);
		auto p_texture   = mat->find_parameter<MaterialParameters::Sampler2D>(Name::texture);
		auto p_mip_level = mat->find_parameter<MaterialParameters::UInt>(mip_level_name);
		auto p_power     = mat->find_parameter<MaterialParameters::Float>(power);

		p_mask->value      = mask;
		p_texture->texture = texture;
		p_texture->sampler = EditorResources::default_sampler;
		p_mip_level->value = mip;
		p_power->value     = pow;

		mat->apply();
		DefaultResources::Buffers::screen_quad->rhi_bind(0);
		rhi->draw(6, 0);

		rhi->submit();
	}

	TextureEditorClient& TextureEditorClient::on_object_parameters_changed(bool reinit)
	{
		if (!m_texture || !m_texture->rhi_texture())
			return *this;

		if (!reinit && m_surface->size() != m_texture->size())
		{
			reinit = true;
		}

		if (reinit)
		{
			m_surface->init(ColorFormat::R8G8B8A8, m_texture->size(m_mip_index));
		}

		Vector4f mask = Vector4f(m_red ? 1.f : 0.f, m_green ? 1.f : 0.f, m_blue ? 1.f : 0.f, m_alpha ? 1.f : 0.f);
		render_thread()->call(render_texture_to_surface, m_surface.ptr(), m_texture.ptr(), m_mip_index, mask, m_pow);
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
		render_texture();

		if (m_live_update)
			on_object_parameters_changed();

		return *this;
	}

	TextureEditorClient& TextureEditorClient::select(Object* object)
	{
		Super::select(object);

		if (m_texture.ptr() == object)
			return *this;

		if (Texture2D* texture = Object::instance_cast<Texture2D>(object))
		{
			m_texture = texture;
			m_properties->update(object);

			call_in_render_thread(
					[self = Pointer(this)]() { self->m_surface->rhi_render_target_view()->clear(Color(0, 0, 0, 1)); });
			on_object_parameters_changed(true);
			m_live_update = texture->is_instance_of<RenderSurface>();
		}
		return *this;
	}

	TextureEditorClient& TextureEditorClient::build_dock(uint32_t dock_id)
	{
		auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);
		ImGui::DockBuilderDockWindow("Texture View###texture", dock_id);
		ImGui::DockBuilderDockWindow(PropertyRenderer::static_name(), dock_id_right);
		return *this;
	}
}// namespace Engine
