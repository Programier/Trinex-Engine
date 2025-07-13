#pragma once
#include <Clients/imgui_client.hpp>
#include <Widgets/imgui_windows.hpp>

namespace Engine
{
	class MaterialEditorClient : public ImGuiViewportClient
	{
		trinex_declare_class(MaterialEditorClient, ImGuiViewportClient);

	protected:
		class ContentBrowser* m_content_browser      = nullptr;
		class ImGuiMaterialPreview* m_preview_window = nullptr;
		class PropertyRenderer* m_properties_window  = nullptr;
		class Material* m_material                   = nullptr;

	public:
		MaterialEditorClient();

		virtual MaterialEditorClient& create_content_browser();
		virtual MaterialEditorClient& create_preview_window();
		virtual MaterialEditorClient& create_properties_window();

		MaterialEditorClient& on_bind_viewport(class RenderViewport* viewport) override;
		MaterialEditorClient& update(float dt) override;
		uint32_t build_dock(uint32_t dock) override;
		MaterialEditorClient& select(Object* object) override;

		inline Material* material() const { return m_material; }
	};
}// namespace Engine
