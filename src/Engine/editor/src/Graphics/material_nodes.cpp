#include <Core/struct.hpp>
#include <Graphics/material_nodes.hpp>
#include <Graphics/visual_material.hpp>

namespace Engine::MaterialNodes
{
#define declare_node(node_name, code)                                                                                            \
    struct node_name : public Node {                                                                                             \
        node_name& init() override                                                                                               \
        {                                                                                                                        \
            {                                                                                                                    \
                code;                                                                                                            \
            }                                                                                                                    \
            return *this;                                                                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        const char* name() const override                                                                                        \
        {                                                                                                                        \
            return #node_name;                                                                                                   \
        }                                                                                                                        \
                                                                                                                                 \
        EnumerateType type() const override                                                                                      \
        {                                                                                                                        \
            return static_cast<EnumerateType>(MaterialNodes::Type::node_name);                                                   \
        }                                                                                                                        \
    };                                                                                                                           \
                                                                                                                                 \
    implement_struct(node_name, Engine::MaterialNodes, Engine::MaterialNodes::Node).push([]() {                                  \
        Struct::static_find(MAKE_ENTITY_FULL_NAME(node_name, Engine::MaterialNodes), true)                                       \
                ->struct_constructor(static_void_constructor_of<node_name>);                                                     \
    })

    declare_node(Sin, {
        input.push_back(material->create_element<InputPin>(this, "In", NodePin::DataType::Float));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float));
    });

    declare_node(Cos, {
        input.push_back(material->create_element<InputPin>(this, "In", NodePin::DataType::Float));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float));
    });

    declare_node(Max, {
        input.push_back(material->create_element<InputPin>(this, "A", NodePin::DataType::Float));
        input.push_back(material->create_element<InputPin>(this, "B", NodePin::DataType::Float));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float));
    });
}// namespace Engine::MaterialNodes
