#include <Core/class.hpp>
#include <Graphics/visual_material.hpp>


namespace Engine
{
    implement_engine_class_default_init(VisualMaterial);


    Node* VisualMaterial::root_node() const
    {
        return _M_root_node;
    }

    const Set<Node*>& VisualMaterial::nodes() const
    {
        return _M_nodes;
    }

    Identifier VisualMaterial::next_id()
    {
        return _M_next_id++;
    }

    VisualMaterial::~VisualMaterial()
    {
        while (!_M_nodes.empty())
        {
            Node* node = *_M_nodes.begin();
            delete node;
            _M_nodes.erase(node);
        }
    }

    VisualMaterialElement& VisualMaterialElement::init()
    {
        return *this;
    }

    VisualMaterialElement::~VisualMaterialElement()
    {}

    NodePin::NodePin(struct Node* node, Name name, DataType data) : data_type(data), name(name), node(node)
    {}

    // Nodes
    Node& Node::init()
    {
        return *this;
    }

    Node::~Node()
    {
        material->_M_nodes.erase(this);

        for (NodePin* pin : input)
        {
            delete pin;
        }

        for (NodePin* pin : output)
        {
            delete pin;
        }

        input.clear();
        output.clear();
    }

    struct SinNode : public Node {
        SinNode& init() override
        {
            input.push_back(material->create_element<InputPin>(this, "Value", NodePin::DataType::Float));
            output.push_back(material->create_element<OutputPin>(this, "Output", NodePin::DataType::Float));
            return *this;
        }

        const char* name() override
        {
            return "Sin";
        }
    };

    VisualMaterial::VisualMaterial()
    {
        _M_root_node = create_element<SinNode>();
        create_element<SinNode>();
        create_element<SinNode>();
        create_element<SinNode>();
    }
}// namespace Engine
