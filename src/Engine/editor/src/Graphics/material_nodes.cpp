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

    bool VertexNode::is_removable() const
    {
        return false;
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

    bool FragmentNode::is_removable() const
    {
        return false;
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


    //////////////////////////// MATH NODES ////////////////////////////

    struct Sin : public MaterialNode {
        declare_material_node();
        Sin()
        {
            inputs.push_back(new FloatInputPin(this, "In", true, 0.f));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->sin(inputs[0]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return inputs[0]->value_type();
        }
    };

    struct Cos : public MaterialNode {
        declare_material_node();
        Cos()
        {
            inputs.push_back(new FloatInputPin(this, "In", true, 0.f));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->cos(inputs[0]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return inputs[0]->value_type();
        }
    };

    struct Tan : public MaterialNode {
        declare_material_node();
        Tan()
        {
            inputs.push_back(new FloatInputPin(this, "In", true, 0.f));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->tan(inputs[0]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return inputs[0]->value_type();
        }
    };

    implement_material_node(Sin, Math);
    implement_material_node(Cos, Math);
    implement_material_node(Tan, Math);

    //////////////////////////// OPERATOR NODES ////////////////////////////

    struct Add : public MaterialNode {
        declare_material_node();
        Add()
        {
            inputs.push_back(new FloatInputPin(this, "A", true, 0.f));
            inputs.push_back(new FloatInputPin(this, "B", true, 0.f));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->add(inputs[0], inputs[1]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return operator_result_between(inputs[0]->value_type(), inputs[1]->value_type());
        }
    };

    struct Sub : public MaterialNode {
        declare_material_node();
        Sub()
        {
            inputs.push_back(new FloatInputPin(this, "A", true, 0.f));
            inputs.push_back(new FloatInputPin(this, "B", true, 0.f));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->sub(inputs[0], inputs[1]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return operator_result_between(inputs[0]->value_type(), inputs[1]->value_type());
        }
    };

    struct Mul : public MaterialNode {
        declare_material_node();
        Mul()
        {
            inputs.push_back(new FloatInputPin(this, "A", true, 0.f));
            inputs.push_back(new FloatInputPin(this, "B", true, 0.f));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->mul(inputs[0], inputs[1]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return operator_result_between(inputs[0]->value_type(), inputs[1]->value_type());
        }
    };

    struct Div : public MaterialNode {
        declare_material_node();
        Div()
        {
            inputs.push_back(new FloatInputPin(this, "A", true, 0.f));
            inputs.push_back(new FloatInputPin(this, "B", true, 0.f));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->div(inputs[0], inputs[1]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return operator_result_between(inputs[0]->value_type(), inputs[1]->value_type());
        }
    };

    implement_material_node(Add, Operators);
    implement_material_node(Sub, Operators);
    implement_material_node(Mul, Operators);
    implement_material_node(Div, Operators);


    //////////////////////////// GLOBALS NODES ////////////////////////////

    struct Time : public MaterialNode {
        declare_material_node();

        Time()
        {
            outputs.push_back(new FloatOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->time();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Float;
        }
    };


    struct DeltaTime : public MaterialNode {
        declare_material_node();

        DeltaTime()
        {
            outputs.push_back(new FloatOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->delta_time();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Float;
        }
    };

    struct Gamma : public MaterialNode {
        declare_material_node();

        Gamma()
        {
            outputs.push_back(new FloatOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->gamma();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Float;
        }
    };

    struct FOV : public MaterialNode {
        declare_material_node();

        FOV()
        {
            outputs.push_back(new FloatOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->fov();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Float;
        }
    };

    struct OrthoWidth : public MaterialNode {
        declare_material_node();

        OrthoWidth()
        {
            outputs.push_back(new FloatOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->ortho_width();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Float;
        }
    };

    struct OrthoHeight : public MaterialNode {
        declare_material_node();

        OrthoHeight()
        {
            outputs.push_back(new FloatOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->ortho_height();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Float;
        }
    };

    struct NearClipPlane : public MaterialNode {
        declare_material_node();

        NearClipPlane()
        {
            outputs.push_back(new FloatOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->near_clip_plane();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Float;
        }
    };

    struct FarClipPlane : public MaterialNode {
        declare_material_node();

        FarClipPlane()
        {
            outputs.push_back(new FloatOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->far_clip_plane();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Float;
        }
    };

    struct AspectRatio : public MaterialNode {
        declare_material_node();

        AspectRatio()
        {
            outputs.push_back(new FloatOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->aspect_ratio();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Float;
        }
    };

    struct CameraProjectionMode : public MaterialNode {
        declare_material_node();

        CameraProjectionMode()
        {
            outputs.push_back(new IntOutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->camera_projection_mode();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Int;
        }
    };

    struct FragCoord : public MaterialNode {
        declare_material_node();

        FragCoord()
        {
            outputs.push_back(new Vec2OutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->frag_coord();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Vec2;
        }
    };

    struct RenderTargetSize : public MaterialNode {
        declare_material_node();

        RenderTargetSize()
        {
            outputs.push_back(new Vec2OutputPin(this, "Out", false));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->render_target_size();
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Vec2;
        }
    };

    implement_material_node(Time, Globals);
    implement_material_node(Gamma, Globals);
    implement_material_node(DeltaTime, Globals);
    implement_material_node(FOV, Globals);
    implement_material_node(OrthoWidth, Globals);
    implement_material_node(OrthoHeight, Globals);
    implement_material_node(NearClipPlane, Globals);
    implement_material_node(FarClipPlane, Globals);
    implement_material_node(AspectRatio, Globals);
    implement_material_node(CameraProjectionMode, Globals);
    implement_material_node(FragCoord, Globals);
    implement_material_node(RenderTargetSize, Globals);

    //////////////////////////// CONSTANT NODES ////////////////////////////

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

    implement_material_node(Float, Constants);

}// namespace Engine::MaterialNodes
