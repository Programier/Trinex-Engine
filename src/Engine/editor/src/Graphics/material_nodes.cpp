#include <Core/struct.hpp>
#include <Graphics/material_nodes.hpp>
#include <Graphics/visual_material.hpp>


namespace Engine::MaterialNodes
{
#define declare_node(node_name, group_name, code)                                                                                \
    struct node_name : public Node {                                                                                             \
        static Struct* node_struct_instance;                                                                                     \
                                                                                                                                 \
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
    Struct* node_name::node_struct_instance = nullptr;                                                                           \
                                                                                                                                 \
    implement_struct(node_name, Engine::MaterialNodes, Engine::MaterialNodes::Node).push([]() {                                  \
        node_name::node_struct_instance = &(Struct::static_find(MAKE_ENTITY_FULL_NAME(node_name, Engine::MaterialNodes), true)   \
                                                    ->struct_constructor(static_void_constructor_of<node_name>)                  \
                                                    .group(#group_name));                                                        \
    })

    declare_node(Sin, Math, {
        input.push_back(material->create_element<FloatInputPin>(this, "In"));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float));
    });

    declare_node(Cos, Math, {
        input.push_back(material->create_element<FloatInputPin>(this, "In"));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::Float));
    });

    declare_node(Max, Math, {
        input.push_back(material->create_element<InputPin>(this, "A", NodePin::DataType::All));
        input.push_back(material->create_element<InputPin>(this, "B", NodePin::DataType::All));
        output.push_back(material->create_element<OutputPin>(this, "Out", NodePin::DataType::All));
    });


#define declare_constant(const_name)                                                                                             \
    declare_node(const_name, Constants, { output.push_back(material->create_element<const_name##OutputPin>(this, "Out")); })

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
    declare_constant(Color);
}// namespace Engine::MaterialNodes
