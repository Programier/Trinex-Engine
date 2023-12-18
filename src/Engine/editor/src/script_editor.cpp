#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine_config.hpp>
#include <Core/file_manager.hpp>
#include <Graphics/imgui.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <Systems/event_system.hpp>
#include <Window/window.hpp>
#include <angelscript.h>
#include <dock_window.hpp>
#include <imgui.h>
#include <script_editor.hpp>
#include <theme.hpp>

namespace Engine
{
    implement_engine_class_default_init(ScriptEditorClient);

    static int input_text_callback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            String* str        = static_cast<String*>(data->UserData);
            const int new_size = data->BufTextLen;
            str->resize(new_size);
            data->Buf = str->data();
        }
        return 0;
    }

    struct ScriptFile {
        String name;
        String buffer;
        ScriptDirectoryNode* node = nullptr;
    };

    struct ScriptDirectoryNode {
        Path path;
        String name;
        Vector<ScriptFile*> files;
        Vector<ScriptDirectoryNode*> subdirectories;

        ~ScriptDirectoryNode()
        {
            for (ScriptFile* file : files)
            {
                delete file;
            }
        }
    };

    static ScriptDirectoryNode* create_script_directory_node(const Path& current)
    {
        ScriptDirectoryNode* current_node = new ScriptDirectoryNode();
        current_node->path                = current;
        current_node->name                = current.filename().string();

        for (auto& entry : FS::directory_iterator(current))
        {
            if (FS::is_regular_file(entry.path()))
            {
                if (entry.path().extension() == Constants::script_extension)
                {
                    ScriptFile* file = new ScriptFile();
                    file->node       = current_node;
                    file->name       = entry.path().filename().string();

                    FileReader* reader = FileManager::root_file_manager()->create_file_reader(entry.path());

                    if (reader)
                    {
                        file->buffer.resize(reader->size());
                        reader->read(reinterpret_cast<byte*>(file->buffer.data()), file->buffer.size());
                        delete reader;
                    }

                    current_node->files.push_back(file);
                }
            }
            else if (FS::is_directory(entry.path()))
            {
                current_node->subdirectories.push_back(create_script_directory_node(entry.path()));
            }
        }

        return current_node;
    }

    ScriptEditorClient& ScriptEditorClient::on_bind_to_viewport(class RenderViewport* viewport)
    {
        Window* window = viewport->window();
        if (window == nullptr)
        {
            throw EngineException("Cannot use ScriptEditorViewport with non-window viewport");
        }

        window->imgui_initialize(initialize_theme);
        _M_root_node = create_script_directory_node(FileManager::root_file_manager()->work_dir() / engine_config.scripts_dir);
        EventSystem::new_system<EventSystem>()->process_event_method(EventSystem::WaitingEvents);

        return *this;
    }

    ScriptEditorClient& ScriptEditorClient::render(class RenderViewport* viewport)
    {
        viewport->window()->rhi_bind();
        viewport->window()->imgui_window()->render();
        return *this;
    }

    ScriptEditorClient& ScriptEditorClient::update(class RenderViewport* viewport, float dt)
    {
        auto* window = viewport->window()->imgui_window();
        window->new_frame();
        make_dock_window();

        ImGui::Begin("Scripts");
        render_scripts_files(_M_root_node);
        ImGui::End();

        render_content();

        window->end_frame();
        return *this;
    }

    ScriptEditorClient& ScriptEditorClient::prepare_render(class RenderViewport* viewport)
    {
        viewport->window()->imgui_window()->prepare_render();
        return *this;
    }

    ScriptEditorClient::~ScriptEditorClient()
    {
        if (_M_root_node)
        {
            delete _M_root_node;
        }
    }

    ScriptEditorClient& ScriptEditorClient::render_scripts_files(ScriptDirectoryNode* node)
    {

        if (ImGui::CollapsingHeader(node->name.c_str()))
        {
            ImGui::Indent(10);
            ImGui::SeparatorText("Directories");
            for (ScriptDirectoryNode* subnode : node->subdirectories)
            {
                render_scripts_files(subnode);
            }

            ImGui::SeparatorText("Files");
            for (ScriptFile* file : node->files)
            {
                if (ImGui::Selectable(file->name.c_str(), _M_selected_file == file))
                {
                    _M_selected_file = file;
                }
            }
            ImGui::Unindent(10);
        }
        return *this;
    }

    ScriptEditorClient& ScriptEditorClient::render_content()
    {
        ImGui::Begin("Content");
        if (_M_selected_file)
        {

            ImGui::InputTextMultiline(
                    "Text", _M_selected_file->buffer.data(), _M_selected_file->buffer.size() + 1, ImGui::GetContentRegionAvail(),
                    ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_AutoSelectAll,
                    input_text_callback, &_M_selected_file->buffer);
        }


        ImGui::End();
        return *this;
    }
}// namespace Engine
