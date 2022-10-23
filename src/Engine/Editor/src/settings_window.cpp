#include <ImGui/imgui.h>
#include <editor.hpp>
#include <fstream>
#include <settings_window.hpp>
#include <vector>


namespace Engine
{

    static ImGuiStyle* style = nullptr;


    static void color_settings()
    {
        static const char* colorNames[] = {
                "ImText",
                "ImTextDisabled",
                "ImWindowBg",
                "ImChildBg",
                "ImPopupBg",
                "ImBorder",
                "ImBorderShadow",
                "ImFrameBg",
                "ImFrameBgHovered",
                "ImFrameBgActive",
                "ImTitleBg",
                "ImTitleBgActive",
                "ImTitleBgCollapsed",
                "ImMenuBarBg",
                "ImScrollbarBg",
                "ImScrollbarGrab",
                "ImScrollbarGrabHovered",
                "ImScrollbarGrabActive",
                "ImCheckMark",
                "ImSliderGrab",
                "ImSliderGrabActive",
                "ImButton",
                "ImButtonHovered",
                "ImButtonActive",
                "ImHeader",
                "ImHeaderHovered",
                "ImHeaderActive",
                "ImSeparator",
                "ImSeparatorHovered",
                "ImSeparatorActive",
                "ImResizeGrip",
                "ImResizeGripHovered",
                "ImResizeGripActive",
                "ImTab",
                "ImTabHovered",
                "ImTabActive",
                "ImTabUnfocused",
                "ImTabUnfocusedActive",
                "ImPlotLines",
                "ImPlotLinesHovered",
                "ImPlotHistogram",
                "ImPlotHistogramHovered",
                "ImTableHeaderBg",
                "ImTableBorderStrong",
                "ImTableBorderLight",
                "ImTableRowBg",
                "ImTableRowBgAlt",
                "ImTextSelectedBg",
                "ImDragDropTarget",
                "ImNavHighlight",
                "ImNavWindowingHighlight",
                "ImNavWindowingDimBg",
                "ImModalWindowDimBg",
        };

        ImGui::Text("ColorChanger");
        ImGui::Separator();
        std::size_t index = 0;

        for (auto name : colorNames)
        {
            ImGui::ColorEdit4(name, reinterpret_cast<float*>(&style->Colors[index++]));
            ImGui::Separator();
        }
    }


#define THEME_ITEM(item_code)                                                                                               \
    item_code;                                                                                                              \
    ImGui::Separator();

    static void theme_settings()
    {
        //float Alpha;         // Global alpha applies to everything in Dear ImGui.
        THEME_ITEM(ImGui::Text("Theme Settings"));
        THEME_ITEM(ImGui::SliderFloat("Alpha Channel", &style->Alpha, 0.01f, 1));
        THEME_ITEM(ImGui::SliderFloat("Disabled Alpha Channel", &style->DisabledAlpha, 0.01f, 1, "%.3f"));
        THEME_ITEM(ImGui::InputFloat2("Window Padding", reinterpret_cast<float*>(&style->WindowPadding)));
        THEME_ITEM(ImGui::SliderFloat("Window Rounding", &style->WindowRounding, 0, 20));
        THEME_ITEM(ImGui::SliderFloat("Window Border size", &style->WindowBorderSize, 0, 4));
        THEME_ITEM(ImGui::SliderFloat("Child Rounding", &style->ChildRounding, 0, 20));
        THEME_ITEM(ImGui::SliderFloat("Child Border size", &style->ChildBorderSize, 0, 4));
        THEME_ITEM(ImGui::SliderFloat("Popup Rounding", &style->PopupRounding, 0, 20));
        THEME_ITEM(ImGui::SliderFloat("Popup Border size", &style->PopupBorderSize, 0, 4));
        THEME_ITEM(ImGui::SliderFloat("Frame Rounding", &style->FrameRounding, 0, 20));
        THEME_ITEM(ImGui::SliderFloat("Frame Border size", &style->FrameBorderSize, 0, 4));
        THEME_ITEM(ImGui::SliderFloat2("Frame Padding", reinterpret_cast<float*>(&style->FramePadding), 0, 10));

        THEME_ITEM({
            auto min_size = style->WindowMinSize;
            if (ImGui::InputFloat2("Window Min Size", reinterpret_cast<float*>(&min_size), "%.3f"))
            {
                if (min_size.x > 0)
                    style->WindowMinSize.x = min_size.x;
                if (min_size.y > 0)
                    style->WindowMinSize.y = min_size.y;
            }
        });

        THEME_ITEM(ImGui::DragFloat2("Window title align", reinterpret_cast<float*>(&style->WindowTitleAlign), 0.001, 0, 1));
        THEME_ITEM(ImGui::DragFloat2("Item Spacing", reinterpret_cast<float*>(&style->ItemSpacing), 0.001, 0, 100));
        THEME_ITEM(
                ImGui::DragFloat2("Item Inner Spacing", reinterpret_cast<float*>(&style->ItemInnerSpacing), 0.001, 0, 100));
        THEME_ITEM(ImGui::SliderFloat("Indent Spacing", &style->IndentSpacing, 0, 100));
        THEME_ITEM(ImGui::SliderFloat("Column Min Spacing", &style->ColumnsMinSpacing, 0, 100));
        THEME_ITEM(ImGui::DragFloat2("Cell Padding", reinterpret_cast<float*>(&style->CellPadding), 0.001, 0, 100));
        THEME_ITEM(ImGui::SliderFloat("ScrollBar Size", &style->ScrollbarSize, 0, 20));
        THEME_ITEM(ImGui::SliderFloat("ScrollBar Rounding", &style->ScrollbarRounding, 0, 20));


        //        ImGuiDir
        //                WindowMenuButtonPosition;// Side of the collapsing/docking button in the title bar (None/Left/Right). Defaults to ImGuiDir_Left.
        //        ImVec2 TouchExtraPadding;// Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
        //        float IndentSpacing;// Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
        //        float ColumnsMinSpacing;// Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
        //        float GrabMinSize;      // Minimum width/height of a grab box for slider/scrollbar.
        //        float GrabRounding;     // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
        //        float LogSliderDeadzone;// The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
        //        float TabRounding;      // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
        //        float TabBorderSize;    // Thickness of border around tabs.
        //        float TabMinWidthForCloseButton;// Minimum width for close button to appears on an unselected tab when hovered. Set to 0.0f to always show when hovering, set to FLT_MAX to never show close button unless selected.
        //        ImGuiDir
        //                ColorButtonPosition;// Side of the color button in the ColorEdit4 widget (left/right). Defaults to ImGuiDir_Right.
        //        ImVec2 ButtonTextAlign;// Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).
        //        ImVec2 SelectableTextAlign;// Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
        //        ImVec2 DisplayWindowPadding;// Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
        //        ImVec2 DisplaySafeAreaPadding;// If you cannot see the edges of your screen (e.g. on a TV) increase the safe area padding. Apply to popups/tooltips as well regular windows. NB: Prefer configuring your TV sets correctly!
        //        float MouseCursorScale;// Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). May be removed later.
        //        bool AntiAliasedLines;// Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
        //        bool AntiAliasedLinesUseTex;// Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering (NOT point/nearest filtering). Latched at the beginning of the frame (copied to ImDrawList).
        //        bool AntiAliasedFill;// Enable anti-aliased edges around filled shapes (rounded rectangles, circles, etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
        //        float CurveTessellationTol;// Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
        //        float CircleTessellationMaxError;// Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.
    }

    static std::vector<std::pair<const char*, void (*)()>> menu = {
            {"Colors", color_settings},
            {"Theme", theme_settings},
    };


    SettingsWindow::SettingsWindow()
    {
        need_render = false;
        style = &ImGui::GetStyle();

        // Read configuration

        std::ifstream config("theme.pcnf", std::ios_base::binary);
        if (config.is_open())
        {
            config.read((char*) style, sizeof(*style));
        }
    }

    void SettingsWindow::render()
    {
        if (!need_render)
            return;

        if (_M_first_frame)
        {
            ImGui::SetNextWindowPos(ImVec2(window_size.x * 0.5f, window_size.y * 0.5f), ImGuiCond_Always, {0.5f, 0.5f});
            ImGui::SetNextWindowSize({window_size.x / 2, window_size.y / 2});
        }

        ImGui::Begin("Setting", &need_render, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Columns(2, nullptr);

        ImGui::BeginChild("Settings.LeftChild");

        for (std::size_t i = 0; i < menu.size(); i++)
        {
            if (ImGui::MenuItem(menu[i].first))
            {
                _M_choiced_index = i;
                break;
            }
        }

        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("Settings.RightChild");
        menu[_M_choiced_index % menu.size()].second();
        ImGui::EndChild();

        ImGui::End();
        if ((_M_first_frame = !need_render))
        {
            // Write config
            std::ofstream("theme.pcnf", std::ios_base::binary).write((char*) style, sizeof(*style));
        }
    }
}// namespace Engine
