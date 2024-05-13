#pragma once
#include <imgui_node_editor.h>

namespace Engine
{
    namespace ed = ax::NodeEditor;

    class NodeBuilder
    {
    public:
        enum class IconType : ImU32
        {
            Flow,
            Circle,
            Square,
            Grid,
            RoundSquare,
            Diamond
        };

    private:
        enum class Stage
        {
            Invalid,
            Begin,
            Header,
            Content,
            Input,
            Output,
            Middle,
            End
        };

        bool transition_to_stage(Stage stage);

        void begin_pin(ed::PinId id, ax::NodeEditor::PinKind kind);
        void end_pin();

        ed::NodeId m_node_id;
        Stage m_stage;
        ImU32 m_header_color;
        ImVec2 m_node_min;
        ImVec2 m_node_max;
        ImVec2 m_header_min;
        ImVec2 m_header_max;
        ImVec2 m_content_min;
        ImVec2 m_content_max;
        bool m_has_header;

    public:
        NodeBuilder();

        void begin(ed::NodeId id);
        void end();
        void begin_header(const ImVec4& color = ImVec4(1, 1, 1, 1));
        void end_header();
        void begin_input(ed::PinId id);
        void end_input();
        void middle();
        void begin_output(ed::PinId id);
        void end_output();

        void icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color = ImVec4(1, 1, 1, 1),
                  const ImVec4& inner_color = ImVec4(0, 0, 0, 0));
    };
}// namespace Engine
