#pragma once
#include <Editor/tab_item.hpp>

namespace Engine
{
    class ContentEditor : public TabItem
    {
    protected:
        bool _M_table_inited = false;

        void render_left_panel();
    public:
        ContentEditor();
        void render() override;
    };
}
