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
        MaterialNodes::Type type() const override                                                                                \
        {                                                                                                                        \
            return MaterialNodes::Type::node_name;                                                                               \
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
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Cos, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Tan, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(ASin, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(ACos, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(ATan, Math::Trigonometric, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });


    declare_node(SinH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(CosH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(TanH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(ASinH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(ACosH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(ATanH, Math::Hyperbolic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Pow, Math::Exponential and Logarithmic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Exp, Math::Exponential and Logarithmic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Log, Math::Exponential and Logarithmic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Exp2, Math::Exponential and Logarithmic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Log2, Math::Exponential and Logarithmic, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Sqrt, Math::Common, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(InverseSqrt, Math::Common, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Abs, Math::Common, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Sign, Math::Common, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Floor, Math::Common, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Ceil, Math::Common, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Fract, Math::Common, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Mod, Math::Common, {
        input.push_back(material->create_element<FloatInputPin>(this, "In", 0));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Max, Math::Common, {
        input.push_back(material->create_element<InputPin>(this, "A", NodePin::DataType::Undefined, 0));
        input.push_back(material->create_element<InputPin>(this, "B", NodePin::DataType::Undefined, 1));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

    declare_node(Min, Math::Common, {
        input.push_back(material->create_element<InputPin>(this, "A", NodePin::DataType::Undefined, 0));
        input.push_back(material->create_element<InputPin>(this, "B", NodePin::DataType::Undefined, 1));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Undefined, 0));
    });

//          Clamp,
//            Mix,
//            Step,
//            Smoothstep


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
