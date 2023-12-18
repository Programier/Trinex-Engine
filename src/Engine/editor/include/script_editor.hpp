#pragma once
#include <Graphics/render_viewport.hpp>
#include <imgui_text_editor.h>

namespace Engine
{

    struct ScriptDirectoryNode;
    struct ScriptFile;

    class ScriptEditorClient : public ViewportClient
    {
        declare_class(ScriptEditorClient, ViewportClient);
        ScriptDirectoryNode* _M_root_node = nullptr;
        ScriptFile* _M_selected_file      = nullptr;
        class RenderViewport* _M_viewport = nullptr;
        ImGui::TextEditor _M_editor;

    public:
        ScriptEditorClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        ScriptEditorClient& render(class RenderViewport* viewport) override;
        ScriptEditorClient& update(class RenderViewport* viewport, float dt) override;
        ScriptEditorClient& prepare_render(class RenderViewport* viewport) override;

        ~ScriptEditorClient();


        ScriptEditorClient& on_file_select(ScriptFile* new_file);
        ScriptEditorClient& render_scripts_files(ScriptDirectoryNode* node);
        ScriptEditorClient& render_content();
    };
}// namespace Engine
