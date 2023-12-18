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
#include <imgui_text_editor.h>
#include <script_editor.hpp>
#include <theme.hpp>

namespace Engine
{
    implement_engine_class_default_init(ScriptEditorClient);

    struct ScriptFile {
        String name;
        String buffer;
        ScriptDirectoryNode* node = nullptr;
        bool is_modified          = false;
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
        if (_M_viewport)
        {
            throw EngineException("This instance of script object alredy used by window");
        }

        Window* window = viewport->window();
        if (window == nullptr)
        {
            throw EngineException("Cannot use ScriptEditorViewport with non-window viewport");
        }

        _M_viewport = viewport;

        window->imgui_initialize(initialize_theme);
        _M_root_node = create_script_directory_node(FileManager::root_file_manager()->work_dir() / engine_config.scripts_dir);
        EventSystem::new_system<EventSystem>()->process_event_method(EventSystem::WaitingEvents);


        auto lang = ImGui::TextEditor::LanguageDefinition::AngelScript();
        _M_editor.SetLanguageDefinition(lang);

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

    ScriptEditorClient& ScriptEditorClient::on_file_select(ScriptFile* new_file)
    {
        if(_M_selected_file && _M_selected_file->is_modified)
        {
            _M_selected_file->buffer = _M_editor.GetText();
        }
        _M_editor.SetText(new_file->buffer);
        _M_selected_file = new_file;
        return *this;
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
                    on_file_select(file);
                }

                if(file->is_modified)
                {
                    ImGui::SameLine();
                    ImGui::Text(" [MODIFIED]");
                }
            }
            ImGui::Unindent(10);
        }
        return *this;
    }

    ScriptEditorClient& ScriptEditorClient::render_content()
    {
        ImGui::Begin("Content");
        _M_editor.Render("Contend", ImGui::GetContentRegionAvail(), true);

        if (_M_selected_file && !_M_selected_file->is_modified)
        {
            _M_selected_file->is_modified = _M_editor.IsTextChanged();
        }

        ImGui::End();
        return *this;
    }
}// namespace Engine
