#include <Compiler/compiler.hpp>
#include <Core/group.hpp>
#include <Core/struct.hpp>
#include <Graphics/material_nodes.hpp>


namespace Engine::MaterialNodes
{

#define declare_material_node()                                                                                                  \
    static Struct* static_struct_instance;                                                                                       \
    Struct* struct_instance() const override;                                                                                    \
    const char* name() const override;


#define implement_material_node(node_name, group_name)                                                                           \
    Struct* node_name::static_struct_instance = nullptr;                                                                         \
    Struct* node_name::struct_instance() const                                                                                   \
    {                                                                                                                            \
        return static_struct_instance;                                                                                           \
    }                                                                                                                            \
    const char* node_name::name() const                                                                                          \
    {                                                                                                                            \
        return #node_name;                                                                                                       \
    }                                                                                                                            \
    implement_struct(node_name, Engine::MaterialNodes, Engine::MaterialNodes::Node).push([]() {                                  \
        node_name::static_struct_instance = &(Struct::static_find(MAKE_ENTITY_FULL_NAME(node_name, Engine::MaterialNodes), true) \
                                                      ->struct_constructor(static_void_constructor_of<node_name>)                \
                                                      .group(Group::find("Engine::VisualMaterialNodes::" #group_name, true)));   \
    });


    VertexNode::VertexNode()
    {
        inputs.push_back(new Vec3InputPin(this, "Position"));
    }

    const char* VertexNode::name() const
    {
        return "Vertex";
    }

    size_t VertexNode::compile(ShaderCompiler* compiler, MaterialOutputPin* pin)
    {
        return 0;
    }

    FragmentNode::FragmentNode()
    {
        inputs.push_back(new Color4InputPin(this, "Base Color"));
        inputs.push_back(new FloatInputPin(this, "Metalic"));
        inputs.push_back(new FloatInputPin(this, "Specular"));
        inputs.push_back(new FloatInputPin(this, "Roughness"));
        inputs.push_back(new Color4InputPin(this, "Emmisive"));
        inputs.push_back(new FloatInputPin(this, "Opacity"));
        inputs.push_back(new FloatInputPin(this, "Opacity Mask"));
        inputs.push_back(new Vec4InputPin(this, "Normal"));
    }

    const char* FragmentNode::name() const
    {
        return "Fragment";
    }

    size_t FragmentNode::compile(ShaderCompiler* compiler, MaterialOutputPin* pin)
    {
        compiler->base_color(inputs[0]);
        return 0;
    }


    Struct* VertexNode::static_struct_instance = nullptr;
    Struct* VertexNode::struct_instance() const
    {
        return static_struct_instance;
    }
    implement_struct(VertexNode, Engine::MaterialNodes, Engine::MaterialNodes::Node).push([]() {
        VertexNode::static_struct_instance = &(Struct::static_find(MAKE_ENTITY_FULL_NAME(VertexNode, Engine::MaterialNodes), true)
                                                       ->struct_constructor(static_void_constructor_of<VertexNode>));
    });

    Struct* FragmentNode::static_struct_instance = nullptr;
    Struct* FragmentNode::struct_instance() const
    {
        return static_struct_instance;
    }
    implement_struct(FragmentNode, Engine::MaterialNodes, Engine::MaterialNodes::Node).push([]() {
        VertexNode::static_struct_instance =
                &(Struct::static_find(MAKE_ENTITY_FULL_NAME(FragmentNode, Engine::MaterialNodes), true)
                          ->struct_constructor(static_void_constructor_of<FragmentNode>));
    });


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct Sin : public MaterialNode {
        declare_material_node();
        Sin()
        {
            inputs.push_back(new FloatInputPin(this, "In", 0.f));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->sin(inputs[0]);
        }
    };

    struct Float : public MaterialNode {
        declare_material_node();

        Float()
        {
            outputs.push_back(new FloatOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->float_constant(*reinterpret_cast<float*>(pin->default_value()));
        }
    };

    implement_material_node(Sin, Math);
    implement_material_node(Float, Constants);

}// namespace Engine::MaterialNodes
