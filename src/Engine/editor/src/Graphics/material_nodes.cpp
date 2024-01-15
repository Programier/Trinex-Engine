#include <Core/struct.hpp>
#include <Graphics/material_nodes.hpp>
#include <Graphics/visual_material.hpp>


namespace Engine::MaterialNodes
{
#define declare_node(node_name, group_name, code)                                                                                \
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
                                                                                                                                 \
        Struct* node_struct() const override                                                                                     \
        {                                                                                                                        \
            return node_struct_instance;                                                                                         \
        }                                                                                                                        \
    };                                                                                                                           \
                                                                                                                                 \
    implement_struct(node_name, Engine::MaterialNodes, Engine::MaterialNodes::Node).push([]() {                                  \
        node_name::node_struct_instance = &(Struct::static_find(MAKE_ENTITY_FULL_NAME(node_name, Engine::MaterialNodes), true)   \
                                                    ->struct_constructor(static_void_constructor_of<node_name>)                  \
                                                    .group(#group_name));                                                        \
    })


    declare_node(Sin, Math, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(Cos, Math, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });


    static EnumerateType minmax_input_type(InputPin* pin)
    {
        InputPin* second_pin = pin->node->input[(pin->index + 1) % 2];

        if (second_pin->linked_to)
        {
            return second_pin->linked_to->node->output_pin_type(second_pin->linked_to);
        }

        return pin->data_types;
    }

    static EnumerateType minmax_output_type(OutputPin* pin)
    {
        for (InputPin* in : pin->node->input)
        {
            if (in->linked_to)
            {
                return in->linked_to->node->output_pin_type(in->linked_to);
            }
        }

        return pin->data_types;
    }

    declare_node(Max, Math, {
        input.push_back(material->create_element<InputPin>(this, "A", NodePin::DataType::All, 0));
        input.push_back(material->create_element<InputPin>(this, "B", NodePin::DataType::All, 1));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::All, 0));

        input_type_callback  = minmax_input_type;
        output_type_callback = minmax_output_type;
    });

    declare_node(Min, Math, {
        input.push_back(material->create_element<InputPin>(this, "A", NodePin::DataType::All, 0));
        input.push_back(material->create_element<InputPin>(this, "B", NodePin::DataType::All, 1));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::All, 0));

        input_type_callback  = minmax_input_type;
        output_type_callback = minmax_output_type;
    });


#define declare_constant(const_name)                                                                                             \
    declare_node(const_name, Constants, { output.push_back(material->create_element<const_name##OutputPin>(this, "Out", 0)); })

    declare_constant(Bool);
    declare_constant(Int);
    declare_constant(UInt);
    declare_constant(Float);
    declare_constant(BVec2);
    declare_constant(BVec3);
    declare_constant(BVec4);
    declare_constant(IVec2);
    declare_constant(IVec3);
    declare_constant(IVec4);
    declare_constant(UVec2);
    declare_constant(UVec3);
    declare_constant(UVec4);
    declare_constant(Vec2);
    declare_constant(Vec3);
    declare_constant(Vec4);
    declare_constant(Color3);
    declare_constant(Color4);
}// namespace Engine::MaterialNodes
