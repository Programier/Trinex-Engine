#include <Clients/imgui_client.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/reflection/class.hpp>
#include <UI/imgui.hpp>

namespace Trinex
{
	void render_test_browser();

	class ImGuiDebug : public ImGuiViewportClient
	{
		trinex_class(ImGuiDebug, ImGuiViewportClient);
		String m_import;

	public:
		ImGuiDebug& update(float dt)
		{
			Super::update(dt);

			ImGui::Begin("Hello World");
			ImGui::InputText("ASSET", m_import);
			if (ImGui::Button("Import"))
			{
				load_object_from_file(m_import);
			}
			ImGui::End();

			return *this;
		}
	};

	trinex_implement_engine_class(ImGuiDebug, 0) {}
}// namespace Trinex
