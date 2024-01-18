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

    bool VisualMaterialElement::is_removable_element()
    {
        return true;
    }

    VisualMaterialElement& VisualMaterialElement::update_id()
    {
        id = reinterpret_cast<Identifier>(this);
        return *this;
    }

    VisualMaterialElement::~VisualMaterialElement()
    {}

    NodePin::NodePin(struct Node* node, Name name, EnumerateType data, Index index)
        : data_types(data), name(name), node(node), index(index)
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

    bool OutputPin::has_one_output_link() const
    {
        return linked_to.size() == 1;
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
    class Struct* Node::node_struct_instance = nullptr;

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


    struct MaterialRootNode : public Node {
        MaterialRootNode& init() override
        {
            input.push_back(material->create_element<Color3InputPin>(this, "Base Color", 0));
            input.push_back(material->create_element<FloatInputPin>(this, "Metalic", 1));
            input.push_back(material->create_element<FloatInputPin>(this, "Specular", 2));
            input.push_back(material->create_element<FloatInputPin>(this, "Roughness", 3));
            input.push_back(material->create_element<Color3InputPin>(this, "Emmisive", 4));
            input.push_back(material->create_element<FloatInputPin>(this, "Opacity", 5));
            input.push_back(material->create_element<FloatInputPin>(this, "Opacity Mask", 6));
            input.push_back(material->create_element<Vec3InputPin>(this, "Normal", 7));

            input.push_back(material->create_element<Vec3InputPin>(this, "World Position Offset", 8));

            return *this;
        }

        const char* name() const override
        {
            return "Material";
        }

        MaterialNodes::Type type() const override
        {
            return MaterialNodes::Type::Root;
        }

        bool is_removable_element() override
        {
            return false;
        }

        Struct* node_struct() const override
        {
            return nullptr;
        }
    };

    VisualMaterial::VisualMaterial()
    {
        _M_root_node = create_element<MaterialRootNode>();
    }
}// namespace Engine
