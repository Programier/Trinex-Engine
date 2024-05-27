#include <Core/file_manager.hpp>
#include <editor_config.hpp>
#include <imgui.h>
#include <theme.hpp>

namespace Engine
{
    float editor_scale_factor()
    {
        return 1.0f;
    }

    float editor_font_size()
    {
        return editor_config.font_size;
    }

    void initialize_theme(ImGuiContext* ctx)
    {
        // Initialize fonts
        auto& io = ImGui::GetIO();

        FileReader reader(editor_config.font_path);

        if (reader.is_open())
        {
            Buffer buffer = reader.read_buffer();
            ImFontConfig config;
            config.FontDataOwnedByAtlas = false;

            io.Fonts->AddFontFromMemoryTTF(buffer.data(), buffer.size(), editor_font_size(), &config,
                                           io.Fonts->GetGlyphRangesCyrillic());
        }

        io.IniFilename = nullptr;
        io.LogFilename = nullptr;


        ImGuiStyle& style = ImGui::GetStyle();

        ImVec4* colors                         = style.Colors;
        colors[ImGuiCol_Text]                  = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.24f, 0.24f, 0.29f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.06f, 0.06f, 0.10f, 0.94f);
        colors[ImGuiCol_Border]                = ImVec4(0.12f, 0.12f, 0.18f, 0.50f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.20f, 0.25f, 0.94f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.12f, 0.75f, 1.00f, 0.40f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.09f, 0.43f, 0.60f, 0.68f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.28f, 0.59f, 0.92f, 1.00f);
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.28f, 0.59f, 0.92f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.37f, 0.60f, 0.82f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.25f, 0.30f, 0.94f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.28f, 0.36f, 0.45f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.28f, 0.36f, 0.45f, 1.00f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.12f, 0.12f, 0.18f, 0.85f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.20f, 0.25f, 0.30f, 0.94f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.28f, 0.36f, 0.45f, 1.00f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.07f, 0.10f, 0.11f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
        colors[ImGuiCol_DockingPreview]        = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);


        style.WindowRounding    = 4.0f;
        style.ChildRounding     = 4.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;


        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize  = 0.0f;
        style.PopupBorderSize  = 0.0f;
        style.FrameBorderSize  = 0.0f;
        style.TabBorderSize    = 0.0f;

        style.AntiAliasedLines = true;
        style.AntiAliasedFill  = true;

        style.ScaleAllSizes(editor_scale_factor());
    }
}// namespace Engine
