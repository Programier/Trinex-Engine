#include <Core/class.hpp>
#include <Graphics/material_nodes.hpp>
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


    VisualMaterial& VisualMaterial::on_element_created(VisualMaterialElement* element)
    {
        element->id       = 0;
        element->material = this;
        element->init();
        element->update_id();
        return *this;
    }

    Node* VisualMaterial::create_node(class Struct* node_struct)
    {
        Node* node = reinterpret_cast<Node*>(node_struct->create_struct());

        if (node == nullptr)
            return nullptr;

        _M_nodes.insert(node);
        on_element_created(node);
        return node;
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

    VisualMaterialElement& VisualMaterialElement::update_id()
    {
        id = reinterpret_cast<Identifier>(this);
        return *this;
    }

    VisualMaterialElement::~VisualMaterialElement()
    {}

    NodePin::NodePin(struct Node* node, Name name, EnumerateType data) : data_type(data), name(name), node(node)
    {}

    void* NodePin::default_value()
    {
        return nullptr;
    }

    bool NodePin::is_input_pin() const
    {
        return false;
    }

    bool NodePin::is_output_pin() const
    {
        return false;
    }

    bool OutputPin::is_output_pin() const
    {
        return true;
    }

    NodePin::PinType OutputPin::type() const
    {
        return NodePin::Output;
    }

    bool InputPin::is_input_pin() const
    {
        return true;
    }

    NodePin::PinType InputPin::type() const
    {
        return NodePin::Input;
    }

    // Nodes
    Node& Node::init()
    {
        return *this;
    }

    Node& Node::update_id()
    {
        VisualMaterialElement::update_id();
        for (NodePin* pin : input)
        {
            pin->update_id();
        }

        for (NodePin* pin : input)
        {
            pin->update_id();
        }

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

    implement_struct(Node, Engine::MaterialNodes, );


    struct GBufferRootNode : public Node {
        GBufferRootNode& init() override
        {
            input.push_back(material->create_element<ColorInputPin>(this, "Albedo"));
            input.push_back(material->create_element<Vec3InputPin>(this, "Position"));
            input.push_back(material->create_element<Vec3InputPin>(this, "Normal"));
            input.push_back(material->create_element<Vec3InputPin>(this, "Specular"));
            return *this;
        }

        const char* name() const override
        {
            return "GBuffer Root";
        }

        EnumerateType type() const override
        {
            return static_cast<EnumerateType>(MaterialNodes::Type::GBufferRoot);
        }

        Struct* node_struct() const override
        {
            return nullptr;
        }
    };

    VisualMaterial::VisualMaterial()
    {
        _M_root_node = create_element<GBufferRootNode>();
    }
}// namespace Engine