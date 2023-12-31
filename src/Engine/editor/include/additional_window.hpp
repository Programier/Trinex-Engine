#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <imgui.h>

namespace Engine
{
    class ImGuiAdditionalWindowList;

    class ImGuiAdditionalWindow
    {
    public:
        ImGuiAdditionalWindowList* list = nullptr;
        size_t frame_number             = 0;

        ImGuiAdditionalWindow();
        delete_copy_constructors(ImGuiAdditionalWindow);

        virtual bool render(class RenderViewport* viewport) = 0;
        FORCE_INLINE virtual ~ImGuiAdditionalWindow(){};
    };


    class ImGuiAdditionalWindowList final
    {
        struct Node {
            ImGuiAdditionalWindow* window = nullptr;
            Node* next                    = nullptr;
            Node* parent                  = nullptr;
        };

        Node* _M_root = nullptr;

        ImGuiAdditionalWindowList& push(ImGuiAdditionalWindow* window);
        Node* destroy(Node* node);

    public:
        ImGuiAdditionalWindowList() = default;
        delete_copy_constructors(ImGuiAdditionalWindowList);

        template<typename Type, typename... Args>
        Type* create(Args&&... args)
        {
            Type* instance = new Type(std::forward<Args>(args)...);
            push(instance);
            return instance;
        }

        ImGuiAdditionalWindowList& render(class RenderViewport* viewport);
        ~ImGuiAdditionalWindowList();
    };
}// namespace Engine
