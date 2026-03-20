#include <Core/editor_config.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/memory.hpp>
#include <UI/imgui.hpp>
#include <UI/theme.hpp>

namespace Trinex::UI
{
	static void register_font(Buffer& buffer, const ImFontConfig& config, const ImWchar* range, f32 size)
	{
		auto& io = ImGui::GetIO();

		void* memory = ImGui::MemAlloc(buffer.size());
		memcpy(memory, buffer.data(), buffer.size());
		io.Fonts->AddFontFromMemoryTTF(memory, buffer.size(), size, &config, range);
	}

	static void register_font(const Path& path, const ImFontConfig& config, const ImWchar* range)
	{
		FileReader reader(path);
		trinex_verify(reader.is_open());

		Buffer buffer = reader.read_buffer();
		register_font(buffer, config, range, Settings::Editor::small_font_size);
		register_font(buffer, config, range, Settings::Editor::normal_font_size);
		register_font(buffer, config, range, Settings::Editor::large_font_size);
	}

	void initialize_theme(ImGuiContext* ctx)
	{
		auto& io = ImGui::GetIO();
		{
			const ImWchar icons_ranges[] = {
			        0x0020,      0x00FF,// ASCII + Latin
			        0x2000,      0x206F,// punctuation
			        0x2190,      0x21FF,// arrows
			        0x2700,      0x27BF,// dingbats
			        ICON_MIN_LC, ICON_MAX_LC, 0,
			};

			ImFontConfig cfg;
			cfg.FontDataOwnedByAtlas = true;

			register_font(Settings::Editor::font_path, cfg, io.Fonts->GetGlyphRangesCyrillic());
			register_font("[content]:/TrinexEditor/fonts/Lucide/lucide.ttf", cfg, icons_ranges);
		}

		io.Fonts->Build();
		io.FontDefault = text_font();
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;

		ImGuiStyle& style = ImGui::GetStyle();

		ImVec4* colors                             = style.Colors;
		colors[ImGuiCol_Text]                      = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
		colors[ImGuiCol_TextDisabled]              = ImVec4(0.24f, 0.24f, 0.29f, 1.00f);
		colors[ImGuiCol_WindowBg]                  = ImVec4(0.06f, 0.06f, 0.10f, 0.94f);
		colors[ImGuiCol_ChildBg]                   = colors[ImGuiCol_ChildBg];
		colors[ImGuiCol_PopupBg]                   = colors[ImGuiCol_PopupBg];
		colors[ImGuiCol_Border]                    = ImVec4(0.12f, 0.12f, 0.18f, 0.50f);
		colors[ImGuiCol_BorderShadow]              = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg]                   = ImVec4(0.20f, 0.20f, 0.25f, 0.94f);
		colors[ImGuiCol_FrameBgHovered]            = ImVec4(0.12f, 0.75f, 1.00f, 0.40f);
		colors[ImGuiCol_FrameBgActive]             = ImVec4(0.09f, 0.43f, 0.60f, 0.68f);
		colors[ImGuiCol_TitleBg]                   = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgActive]             = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]          = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg]                 = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]               = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab]             = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered]      = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]       = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark]                 = ImVec4(0.28f, 0.59f, 0.92f, 1.00f);
		colors[ImGuiCol_SliderGrab]                = ImVec4(0.28f, 0.59f, 0.92f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]          = ImVec4(0.37f, 0.60f, 0.82f, 1.00f);
		colors[ImGuiCol_Button]                    = ImVec4(0.20f, 0.25f, 0.30f, 0.94f);
		colors[ImGuiCol_ButtonHovered]             = ImVec4(0.28f, 0.36f, 0.45f, 1.00f);
		colors[ImGuiCol_ButtonActive]              = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		colors[ImGuiCol_Header]                    = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
		colors[ImGuiCol_HeaderHovered]             = ImVec4(0.28f, 0.36f, 0.45f, 1.00f);
		colors[ImGuiCol_HeaderActive]              = ImVec4(0.15f, 0.40f, 0.70f, 1.00f);
		colors[ImGuiCol_Separator]                 = ImVec4(0.12f, 0.12f, 0.18f, 0.85f);
		colors[ImGuiCol_SeparatorHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		colors[ImGuiCol_SeparatorActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_ResizeGrip]                = ImVec4(0.20f, 0.25f, 0.30f, 0.94f);
		colors[ImGuiCol_ResizeGripHovered]         = ImVec4(0.28f, 0.36f, 0.45f, 1.00f);
		colors[ImGuiCol_ResizeGripActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		colors[ImGuiCol_InputTextCursor]           = colors[ImGuiCol_InputTextCursor];
		colors[ImGuiCol_Tab]                       = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
		colors[ImGuiCol_TabHovered]                = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		colors[ImGuiCol_TabSelected]               = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
		colors[ImGuiCol_TabDimmed]                 = ImVec4(0.07f, 0.10f, 0.11f, 1.00f);
		colors[ImGuiCol_TabDimmedSelected]         = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
		colors[ImGuiCol_TabSelectedOverline]       = colors[ImGuiCol_TabSelectedOverline];
		colors[ImGuiCol_TabDimmedSelectedOverline] = colors[ImGuiCol_TabDimmedSelectedOverline];
		colors[ImGuiCol_DockingPreview]            = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg]            = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
		colors[ImGuiCol_PlotLines]                 = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered]          = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram]             = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered]      = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg]             = colors[ImGuiCol_TableHeaderBg];
		colors[ImGuiCol_TableBorderStrong]         = colors[ImGuiCol_TableBorderStrong];
		colors[ImGuiCol_TableBorderLight]          = colors[ImGuiCol_TableBorderLight];
		colors[ImGuiCol_TableRowBg]                = colors[ImGuiCol_TableRowBg];
		colors[ImGuiCol_TableRowBgAlt]             = colors[ImGuiCol_TableRowBgAlt];
		colors[ImGuiCol_TextLink]                  = colors[ImGuiCol_TextLink];
		colors[ImGuiCol_TextSelectedBg]            = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_TreeLines]                 = colors[ImGuiCol_TreeLines];
		colors[ImGuiCol_DragDropTarget]            = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavCursor]                 = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight]     = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]         = colors[ImGuiCol_NavWindowingDimBg];
		colors[ImGuiCol_ModalWindowDimBg]          = colors[ImGuiCol_ModalWindowDimBg];


		style.WindowRounding    = 1.0f;
		style.ChildRounding     = 1.0f;
		style.FrameRounding     = 1.0f;
		style.PopupRounding     = 1.0f;
		style.ScrollbarRounding = 1.0f;
		style.GrabRounding      = 1.0f;
		style.TabRounding       = 1.0f;

		style.WindowBorderSize = 0.0f;
		style.ChildBorderSize  = 0.0f;
		style.PopupBorderSize  = 0.0f;
		style.FrameBorderSize  = 0.0f;
		style.TabBorderSize    = 0.0f;

		style.AntiAliasedLines = true;
		style.AntiAliasedFill  = true;
	}

	ImFont* text_font(FontSize size)
	{
		return ImGui::GetIO().Fonts->Fonts[static_cast<u32>(size)];
	}

	ImFont* icons_font(FontSize size)
	{
		return ImGui::GetIO().Fonts->Fonts[3 + static_cast<u32>(size)];
	}
}// namespace Trinex::UI
