#pragma once
#include <Graphics/material.hpp>

namespace Engine
{

    struct VisualMaterialElement {
        Identifier id;
        class VisualMaterial* material = nullptr;

        virtual VisualMaterialElement& init();
        virtual ~VisualMaterialElement();
    };

    struct NodePin : VisualMaterialElement {
        enum DataType : byte
        {
            Float = 0,
            Int   = 1,
        } data_type;

        Name name;
        struct Node* node = nullptr;

        NodePin(struct Node*, Name name, DataType data);
    };

    struct OutputPin : public NodePin {
        using NodePin::NodePin;
    };

    struct InputPin : public NodePin {
        struct OutputPin* linked_to = nullptr;
        using NodePin::NodePin;
    };


    struct Node : VisualMaterialElement {
        Vector<InputPin*> input;
        Vector<OutputPin*> output;

        Node& init();
        virtual const char* name() const   = 0;
        virtual EnumerateType type() const = 0;
        virtual ~Node();
    };

    class VisualMaterial : public Material
    {
        declare_class(VisualMaterial, Material);

    private:
        Identifier _M_next_id = 0;

        Node* _M_root_node = nullptr;
        Set<Node*> _M_nodes;


        VisualMaterial& on_element_created(VisualMaterialElement* element);

    public:
        Node* root_node() const;
        const Set<Node*>& nodes() const;
        Identifier next_id();
        Node* create_node(class Struct*);


        template<typename Type, typename... Args>
        Type* create_element(Args&&... args)
        {
            Type* element = new Type(std::forward<Args>(args)...);
            if constexpr (std::is_base_of_v<Node, Type>)
            {
                _M_nodes.insert(element);
            }
            on_element_created(element);
            return element;
        }

        VisualMaterial();
        ~VisualMaterial();

        friend struct Node;
    };

}// namespace Engine
