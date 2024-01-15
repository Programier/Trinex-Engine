#include <Core/group.hpp>
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
                                                    .group(Group::find("Engine::VisualMaterialNodes::" #group_name, true)));     \
    })


    declare_node(Sin, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(Cos, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(Tan, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(ASin, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(ACos, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(ATan, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "X", 0));
        input.push_back(material->create_element<FloatInputPin>(this, "Y", 1, 1.f));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });


    declare_node(SinH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(CosH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(TanH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(ASinH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(ACosH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(ATanH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(Pow, Math::Exponential and Logarithmic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(Exp, Math::Exponential and Logarithmic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(Log, Math::Exponential and Logarithmic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(Exp2, Math::Exponential and Logarithmic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float, 0));
    });

    declare_node(Log2, Math::Exponential and Logarithmic, {
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

        if (!pin->node->output[0]->linked_to.empty())
        {
            InputPin* in = *pin->node->output[0]->linked_to.begin();
            return in->node->input_pin_type(in);
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

        if (!pin->linked_to.empty())
        {
            InputPin* in = *pin->linked_to.begin();
            return in->node->input_pin_type(in);
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


#define declare_constant(const_name, group_name)                                                                                 \
    declare_node(const_name, group_name, { output.push_back(material->create_element<const_name##OutputPin>(this, "Out", 0)); })

    declare_constant(Bool, Constants);
    declare_constant(Int, Constants);
    declare_constant(UInt, Constants);
    declare_constant(Float, Constants);
    declare_constant(BVec2, Constants::Vectors::Bool);
    declare_constant(BVec3, Constants::Vectors::Bool);
    declare_constant(BVec4, Constants::Vectors::Bool);
    declare_constant(IVec2, Constants::Vectors::Int);
    declare_constant(IVec3, Constants::Vectors::Int);
    declare_constant(IVec4, Constants::Vectors::Int);
    declare_constant(UVec2, Constants::Vectors::UInt);
    declare_constant(UVec3, Constants::Vectors::UInt);
    declare_constant(UVec4, Constants::Vectors::UInt);
    declare_constant(Vec2, Constants::Vectors::Float);
    declare_constant(Vec3, Constants::Vectors::Float);
    declare_constant(Vec4, Constants::Vectors::Float);
    declare_constant(Color3, Constants::Colors);
    declare_constant(Color4, Constants::Colors);
}// namespace Engine::MaterialNodes
