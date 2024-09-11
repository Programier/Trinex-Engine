#include <Clients/texture_editor_client.hpp>
#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	implement_engine_class(TextureEditorClient, 0)
	{
		register_client(Texture::static_class_instance(), static_class_instance());
	}

	static Vector2D max_texture_size_in_viewport(const Vector2D& texture_size, const Vector2D& viewport_size)
	{
		float texture_aspect_ratio  = texture_size.x / texture_size.y;
		float viewport_aspect_ratio = viewport_size.x / viewport_size.y;

		Vector2D result_size;

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
		m_sampler = Object::new_instance<Sampler>();
		m_sampler->init_resource();
	}

	void TextureEditorClient::render_dock()
	{
		auto dock_id                       = ImGui::GetID("TextureEditorDock##Dock");
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags | ImGuiDockNodeFlags_NoTabBar);

		if (imgui_window()->frame_index() == 1)
		{

			ImGui::DockBuilderRemoveNode(dock_id);
			ImGui::DockBuilderAddNode(dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);

			auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);

			ImGui::DockBuilderDockWindow("###texture", dock_id);
			ImGui::DockBuilderDockWindow("###properties", dock_id_right);

			ImGui::DockBuilderFinish(dock_id);
		}
	}

	void TextureEditorClient::render_texture()
	{
		ImGui::Begin("###texture");
		if (m_texture)
		{
			ImGui::BeginHorizontal(0, ImGui::GetContentRegionAvail());
			ImGui::Spring(1.f, 0.5);
			auto size = max_texture_size_in_viewport(m_texture->size(), ImGui::EngineVecFrom(ImGui::GetContentRegionAvail()));
			ImGui::Image(ImTextureID(m_texture.ptr(), m_sampler.ptr()), ImGui::ImVecFrom(size));
			ImGui::Spring(1.f, 0.5);
			ImGui::EndHorizontal();
		}
		ImGui::End();
	}

	void TextureEditorClient::render_properties()
	{
		ImGui::Begin("###properties");
		ImGui::End();
	}

	TextureEditorClient& TextureEditorClient::update(float dt)
	{
		Super::update(dt);

		ImGuiViewport* imgui_viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(imgui_viewport->WorkPos);
		ImGui::SetNextWindowSize(imgui_viewport->WorkSize);

		ImGui::Begin("Texture Editor", nullptr,
		             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
		                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus |
		                     ImGuiWindowFlags_MenuBar);

		render_dock();
		render_texture();
		render_properties();

		ImGui::End();

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
		}
		return *this;
	}
}// namespace Engine
