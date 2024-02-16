#include <Compiler/compiler.hpp>
#include <Core/group.hpp>
#include <Core/struct.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material_nodes.hpp>
#include <icons.hpp>
#include <imgui.h>


namespace Engine::MaterialNodes
{
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


    template<typename InputType, typename OutputType>
    struct ConstructVec2Base : public MaterialNode {

        ConstructVec2Base()
        {
            inputs.push_back(new InputType(this, "X", true, typename InputType::NativeType(0)));
            inputs.push_back(new InputType(this, "Y", true, typename InputType::NativeType(0)));
            outputs.push_back(new OutputType(this, "Out"));
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return OutputType::data_type;
        }
    };

    template<typename InputType, typename OutputType>
    struct ConstructVec3Base : public ConstructVec2Base<InputType, OutputType> {
        ConstructVec3Base()
        {
            ConstructVec2Base<InputType, OutputType>::inputs.push_back(
                    new InputType(this, "Z", true, typename InputType::NativeType(0)));
        }
    };

    template<typename InputType, typename OutputType>
    struct ConstructVec4Base : public ConstructVec3Base<InputType, OutputType> {

        ConstructVec4Base()
        {
            ConstructVec3Base<InputType, OutputType>::inputs.push_back(
                    new InputType(this, "W", true, typename InputType::NativeType(1)));
        }
    };

    template<typename InputType>
    struct DecomposeVecBase : public MaterialNode {
        DecomposeVecBase()
        {
            inputs.push_back(new InputType(this, "Vec", true, typename InputType::NativeType(0)));
            outputs.push_back(new MaterialOutputPin(this, "X"));
            outputs.push_back(new MaterialOutputPin(this, "Y"));
            outputs.push_back(new MaterialOutputPin(this, "Z"));
            outputs.push_back(new MaterialOutputPin(this, "W"));
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            MaterialNodeDataType input_type = reinterpret_cast<MaterialInputPin*>(inputs[0])->value_type();
            MaterialDataTypeInfo info       = MaterialDataTypeInfo::from(input_type);
            return static_cast<MaterialNodeDataType>(material_type_value(info.base_type, 1));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            Index index = 0;
            for (MaterialOutputPin* out_pin : outputs)
            {
                if (out_pin == pin)
                {
                    break;
                }
                ++index;
            }

            if (index < 4)
                return compiler->decompose_vec(inputs[0], static_cast<DecomposeVectorComponent>(index));
            return compile_error;
        }
    };

    struct ConstructVec2 : public ConstructVec2Base<FloatInputPin, Vec4OutputNoDefaultPin> {
        declare_material_node();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->construct_vec2(inputs[0], inputs[1]);
        }
    };

    struct ConstructVec3 : public ConstructVec3Base<FloatInputPin, Vec4OutputNoDefaultPin> {
        declare_material_node();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->construct_vec3(inputs[0], inputs[1], inputs[2]);
        }
    };

    struct ConstructVec4 : public ConstructVec4Base<FloatInputPin, Vec4OutputNoDefaultPin> {
        declare_material_node();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->construct_vec4(inputs[0], inputs[1], inputs[2], inputs[3]);
        }
    };

    struct DecomposeVec : public DecomposeVecBase<Vec4InputPin> {
        declare_material_node();
    };


    implement_material_node(Add, Operators);
    implement_material_node(Sub, Operators);
    implement_material_node(Mul, Operators);
    implement_material_node(Div, Operators);
    implement_material_node(ConstructVec2, Operators);
    implement_material_node(ConstructVec3, Operators);
    implement_material_node(ConstructVec4, Operators);
    implement_material_node(DecomposeVec, Operators);


    //////////////////////////// GLOBALS NODES ////////////////////////////

    struct Time : public MaterialNode {
        declare_material_node();

        Time()
        {
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new IntOutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new Vec2OutputNoDefaultPin(this, "Out"));
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
            outputs.push_back(new Vec2OutputNoDefaultPin(this, "Out"));
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


#define declare_constant_type(name, func_name)                                                                                   \
    struct name : public MaterialNode {                                                                                          \
        declare_material_node();                                                                                                 \
        name()                                                                                                                   \
        {                                                                                                                        \
            outputs.push_back(new name##OutputPin(this, "Out"));                                                                 \
        }                                                                                                                        \
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override                                                \
        {                                                                                                                        \
            return compiler->func_name##_constant(pin->default_value());                                                         \
        }                                                                                                                        \
        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override                                            \
        {                                                                                                                        \
            return MaterialNodeDataType::name;                                                                                   \
        }                                                                                                                        \
    };                                                                                                                           \
    implement_material_node(name, Constants);

    declare_constant_type(Bool, bool);
    declare_constant_type(Int, int);
    declare_constant_type(UInt, uint);
    declare_constant_type(Float, float);
    declare_constant_type(BVec2, bvec2);
    declare_constant_type(BVec3, bvec3);
    declare_constant_type(BVec4, bvec4);
    declare_constant_type(IVec2, ivec2);
    declare_constant_type(IVec3, ivec3);
    declare_constant_type(IVec4, ivec4);
    declare_constant_type(UVec2, uvec2);
    declare_constant_type(UVec3, uvec3);
    declare_constant_type(UVec4, uvec4);
    declare_constant_type(Vec2, vec2);
    declare_constant_type(Vec3, vec3);
    declare_constant_type(Vec4, vec4);
    declare_constant_type(Color3, color3);
    declare_constant_type(Color4, color4);

    //////////////////////////// TEXTURE NODES ////////////////////////////


    Sampler::Sampler()
    {
        outputs.push_back(new SamplerOutputNoDefaultPin(this, "Out"));
    }

    size_t Sampler::compile(ShaderCompiler* compiler, MaterialOutputPin* pin)
    {
        return compiler->sampler(sampler);
    }

    Texture2D::Texture2D()
    {
        outputs.push_back(new Color4OutputNoDefaultPin(this, "Color"));
        inputs.push_back(new MaterialInputPin(this, "Sampler"));
        inputs.push_back(new MaterialInputPin(this, "UV"));
    }

    void Texture2D::render()
    {
        if (texture)
        {
            Engine::Sampler* sampler = Icons::default_sampler();
            ImGuiRenderer::ImGuiTexture* imgui_texture =
                    ImGuiRenderer::Window::current()->create_texture(reinterpret_cast<class Engine::Texture*>(texture), sampler);
            ImGui::Text("%s", reinterpret_cast<Object*>(texture)->string_name().c_str());
            ImGui::Image(imgui_texture->handle(), {100, 100});
        }
    }

    size_t Texture2D::compile(ShaderCompiler* compiler, MaterialOutputPin* pin)
    {
        return compiler->texture_2d(texture, inputs[0], inputs[1]);
    }

    implement_material_node(Sampler, Texture);
    implement_material_node(Texture2D, Texture);
}// namespace Engine::MaterialNodes
