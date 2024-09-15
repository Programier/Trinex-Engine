#include <Core/filesystem/root_filesystem.hpp>
#include <Widgets/file_dialog.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>


#include <Clients/imgui_client.hpp>
#include <Core/class.hpp>
#include <Graphics/render_viewport.hpp>


namespace Engine
{
	static constexpr inline float row_height = 30.f;

	void ImGuiFileDialog::init(RenderViewport* viewport)
	{
		name = "File Dialog";
	}

	ImGuiFileDialog& ImGuiFileDialog::setup_dock_space()
	{
		ImGuiID dockspace_id = ImGui::GetID("Dock");
		ImGui::DockSpace(dockspace_id, {}, (int_t) ImGuiDockNodeFlags_PassthruCentralNode | (int_t) ImGuiDockNodeFlags_NoTabBar);

		if (frame_number == 1)
		{
			auto work_size = ImGui::GetWindowSize();

			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None);
			ImGui::DockBuilderSetNodeSize(dockspace_id, work_size);

			float header = row_height / work_size.y;
			float footer = (row_height * 2.f) / (work_size.y - row_height);

			auto unresizable = [](ImGuiID id) -> ImGuiID {
				if (auto node = ImGui::DockBuilderGetNode(id))
				{
					node->SetLocalFlags(node->LocalFlags | ImGuiDockNodeFlags_NoResize);
				}
				return id;
			};

			auto split = &ImGui::DockBuilderSplitNode;

			ImGuiID dock_id_header = unresizable(split(dockspace_id, ImGuiDir_Up, header, nullptr, &dockspace_id));
			ImGuiID dock_id_footer = unresizable(split(dockspace_id, ImGuiDir_Down, footer, nullptr, &dockspace_id));
			ImGuiID dock_id_left   = split(dockspace_id, ImGuiDir_Left, 0.2, nullptr, &dockspace_id);


			ImGui::DockBuilderDockWindow("###Header", dock_id_header);
			ImGui::DockBuilderDockWindow("###Footer", dock_id_footer);
			ImGui::DockBuilderDockWindow("###Left", dock_id_left);
			ImGui::DockBuilderDockWindow("###Content", dockspace_id);

			ImGui::DockBuilderFinish(dockspace_id);
		}

		return *this;
	}

	ImGuiFileDialog& ImGuiFileDialog::render_header()
	{
		ImGui::Begin("###Header");

		ImGui::BeginHorizontal(0, ImGui::GetContentRegionAvail());

		ImGui::Spring(0);
		for (int i = 0; i < 5; ++i)
		{
			ImGui::Text("%d", i);
		}
		ImGui::Spring(1);

		for (int i = 0; i < 5; ++i)
		{
			ImGui::Text("%d", i);
		}
		ImGui::Spring(0);

		ImGui::EndHorizontal();

		ImGui::End();
		return *this;
	}

	ImGuiFileDialog& ImGuiFileDialog::render_left_panel()
	{
		ImGui::Begin("###Left");

		if (ImGui::CollapsingHeader("editor/Devices"_localized))
		{
			for(auto& [mount, fs] : rootfs()->filesystems())
			{
				if(ImGui::Button(fs.name.c_str()))
				{
					
				}
			}
		}

		ImGui::End();
		return *this;
	}

	ImGuiFileDialog& ImGuiFileDialog::render_content()
	{
		ImGui::Begin("###Content");
		ImGui::End();
		return *this;
	}

	ImGuiFileDialog& ImGuiFileDialog::render_footer()
	{
		ImGui::Begin("###Footer");
		ImGui::End();
		return *this;
	}

	ImGuiFileDialog& ImGuiFileDialog::change_path(const Path& new_path)
	{
		return *this;
	}


	bool ImGuiFileDialog::render(RenderViewport* viewport)
	{
		bool is_open = true;

		if (frame_number == 1)
		{
			ImGui::SetNextWindowSizeConstraints({800, 400}, {FLT_MAX, FLT_MAX});
		}

		if (ImGui::Begin(name.c_str(), &is_open))
			setup_dock_space().render_header().render_left_panel().render_content().render_footer();

		ImGui::End();
		return is_open;
	}


	class FileDialogClient : public ImGuiEditorClient
	{
		declare_class(FileDialogClient, ImGuiEditorClient);

	public:
		FileDialogClient& on_bind_viewport(RenderViewport* vp) override
		{
			Super::on_bind_viewport(vp);
			auto current = ImGuiWindow::current();
			ImGuiWindow::make_current(imgui_window());
			imgui_window()->widgets_list.create<ImGuiFileDialog>();
			ImGuiWindow::make_current(current);
			return *this;
		}
	};

	implement_engine_class_default_init(FileDialogClient, 0);

}// namespace Engine
