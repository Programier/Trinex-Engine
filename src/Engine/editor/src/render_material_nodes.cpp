#include <Graphics/visual_material.hpp>
#include <imgui_node_editor.h>

namespace Engine
{
    namespace ed = ax::NodeEditor;

    void render_material_nodes(Object* object, void* editor_context)
    {
        SetCurrentEditor(reinterpret_cast<ed::EditorContext*>(editor_context));

        ed::Begin("Viewport");
        ed::End();

        ed::SetCurrentEditor(nullptr);
    }
}// namespace Engine
