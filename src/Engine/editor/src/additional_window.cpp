#include <additional_window.hpp>


namespace Engine
{
    ImGuiAdditionalWindow::ImGuiAdditionalWindow()
    {}

    ImGuiAdditionalWindowList& ImGuiAdditionalWindowList::render(class RenderViewport* viewport)
    {
        Node* node = _M_root;

        while (node)
        {
            bool status = node->window->render(viewport);
            node->window->frame_number += 1;
            if (status)
            {
                node = node->next;
            }
            else
            {
                node = destroy(node);
            }
        }

        return *this;
    }

    ImGuiAdditionalWindowList::Node* ImGuiAdditionalWindowList::destroy(Node* node)
    {
        if (node == _M_root)
        {
            _M_root = _M_root->next;
        }

        if (node->parent)
        {
            node->parent->next = node->next;
        }

        if (node->next)
        {
            node->next->parent = node->parent;
        }

        delete node->window;

        Node* next = node->next;
        delete node;

        return next;
    }

    ImGuiAdditionalWindowList& ImGuiAdditionalWindowList::push(ImGuiAdditionalWindow* window)
    {

        Node* parent_node = _M_root;
        while (parent_node && parent_node->next)
            parent_node = parent_node->next;

        window->list = this;
        Node* node   = new Node();
        node->window = window;
        node->parent   = parent_node;
        node->next = nullptr;

        if(parent_node)
        {
            parent_node->next = node;
        }

        if(_M_root == nullptr)
        {
            _M_root = node;
        }

        return *this;
    }

    ImGuiAdditionalWindowList::~ImGuiAdditionalWindowList()
    {
        while (_M_root)
        {
            destroy(_M_root);
        }
    }
}// namespace Engine
