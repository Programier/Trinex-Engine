#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <Image/image.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <Window/window.hpp>
#include <viewport_client.hpp>


namespace Engine
{
    implement_engine_class_default_init(EditorViewportClient);
    ScriptFunction script_viewport_update;

    static void initialize_theme(ImGuiContext* context)
    {
        // Initialize fonts
        const char* font_path = "/usr/share/fonts/adobe-source-code-pro-fonts/SourceCodePro-Bold.otf";
        auto& io              = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF(font_path, 14.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());

        ImGuiStyle& style = ImGui::GetStyle();

        // Налаштування кольорів
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
        colors[ImGuiCol_Header]                = ImVec4(0.20f, 0.25f, 0.30f, 0.94f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.28f, 0.36f, 0.45f, 1.00f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator]             = colors[ImGuiCol_Border];
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

        // Згладжені кути
        style.WindowRounding    = 4.0f;
        style.ChildRounding     = 4.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;

        // Згладження ліній
        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize  = 0.0f;
        style.PopupBorderSize  = 0.0f;
        style.FrameBorderSize  = 0.0f;
        style.TabBorderSize    = 0.0f;

        // Активація анти-аліасінгу
        style.AntiAliasedLines = true;
        style.AntiAliasedFill  = true;
    }


    EditorViewportClient::EditorViewportClient()
    {
        Package* package = Package::find_package("TestResources", true);
        package->load();
        texture = package->find_object_checked<Texture2D>("Trinex", false);
        sampler = package->find_object_checked<Sampler>("Trinex Sampler", false);
        sampler->init_resource();
    }

    ViewportClient& EditorViewportClient::on_bind_to_viewport(class RenderViewport* viewport)
    {
        Window* window = viewport->window();
        if (window == nullptr)
        {
            throw EngineException("Cannot bind client to non-window viewport!");
        }

        window->imgui_initialize(initialize_theme);

        // Update window name
        String new_title = window->title() + Strings::format(" [{} RHI]", engine_instance->rhi()->name().c_str());
        window->title(new_title);

        _M_imgui_texture = window->imgui_window()->create_texture();
        _M_imgui_texture->init(window->imgui_window()->context(), texture, sampler);
        engine_instance->thread(ThreadType::RenderThread)->wait_all();


        auto module = ScriptEngine::instance()->new_module("ViewportScript");

        {
            FileReader* reader = FileManager::root_file_manager()->create_file_reader(Path(engine_config.scripts_dir) /
                                                                                      "viewport.cpp");
            if (reader)
            {
                auto size = reader->size();
                String data(size, ' ');
                reader->read((byte*)data.data(), size);

                module.add_script_section("test_section", data.c_str());
                module.build();

                script_viewport_update = module.function_by_name("update");
                delete reader;
            }
        }

        return *this;
    }

    ViewportClient& EditorViewportClient::render(class RenderViewport* viewport)
    {
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }

    ViewportClient& EditorViewportClient::prepare_render(class RenderViewport* viewport)
    {
        viewport->window()->imgui_window()->prepare_render();
        return *this;
    }


    ViewportClient& EditorViewportClient::update(class RenderViewport* viewport, float dt)
    {
        ImGuiRenderer::Window* window = viewport->window()->imgui_window();
        window->new_frame();
        create_docking_window(dt);
        create_scene_tree_window(dt);
        create_properties_window(dt);
        create_log_window(dt);
        create_viewport_window(dt);

        if (script_viewport_update.is_valid())
        {
            script_viewport_update.prepare().call();
        }

        window->end_frame();

        return *this;
    }

    EditorViewportClient& EditorViewportClient::create_docking_window(float dt)
    {

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);


        static ImGuiWindowFlags dock_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse |
                                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus;

        if (ImGui::Begin("EditorDockWindow", nullptr, dock_flags))
        {
            auto id                            = ImGui::GetID("EditorDockWindow##Dock");
            ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
            ImGui::DockSpace(id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        ImGui::End();
        return *this;
    }

    EditorViewportClient& EditorViewportClient::create_properties_window(float dt)
    {
        if (!ImGui::Begin("Properties"))
        {
            ImGui::End();
            return *this;
        }

        ImGui::Text("FPS: %f", 1.0 / dt);

        ImGui::End();
        return *this;
    }

    static void render_objects_tree(Package* package)
    {
        if (ImGui::TreeNode(package->string_name().c_str()))
        {

            ImGui::Indent(10.f);
            for (auto& [name, object] : package->objects())
            {
                Package* new_package = object->instance_cast<Package>();
                if (new_package)
                {
                    render_objects_tree(new_package);
                }
                else
                {
                    ImGui::Text("%s", object->string_name().c_str());
                }
            }

            ImGui::Unindent(10.f);
            ImGui::TreePop();
        }
    }

    EditorViewportClient& EditorViewportClient::create_scene_tree_window(float dt)
    {
        if (!ImGui::Begin("Scene Tree"))
        {
            ImGui::End();
            return *this;
        }

        render_objects_tree(Object::root_package());

        ImGui::End();

        return *this;
    }

    EditorViewportClient& EditorViewportClient::create_log_window(float dt)
    {
        if (!ImGui::Begin("Logs"))
        {
            ImGui::End();
            return *this;
        }

        ImGui::End();
        return *this;
    }

    EditorViewportClient& EditorViewportClient::create_viewport_window(float dt)
    {
        if (!ImGui::Begin("Viewport", nullptr))
        {
            ImGui::End();
            return *this;
        };

        auto size = ImGui::GetContentRegionAvail();
        ImGui::Image(_M_imgui_texture->handle(), size);
        ImGui::End();

        return *this;
    }

    EditorViewportClient& EditorViewportClient::create_bar(float dt)
    {
        return *this;
    }
}// namespace Engine
