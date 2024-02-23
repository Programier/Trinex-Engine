#include <Compiler/compiler.hpp>
#include <Core/group.hpp>
#include <Core/struct.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material_nodes.hpp>
#include <Graphics/texture_2D.hpp>
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
        inputs.push_back(new Vec3InputNoDefaultPin(this, "Screen Space Position"));
        inputs.push_back(new Vec3InputPin(this, "World Position"));
        inputs.push_back(new Vec2InputPin(this, "UV0"));
        inputs.push_back(new Vec2InputPin(this, "UV1"));
        inputs.push_back(new Vec3InputPin(this, "World Normal"));
        inputs.push_back(new Vec3InputPin(this, "World Tangent"));
        inputs.push_back(new Vec3InputPin(this, "World Bitangent"));
        inputs.push_back(new Vec3InputPin(this, "Vertex Color"));
    }

    const char* VertexNode::name() const
    {
        return "Vertex";
    }

    size_t VertexNode::compile(ShaderCompiler* compiler, MaterialOutputPin* pin)
    {
        compiler->vertex_output_screen_space_position(inputs[0]);
        compiler->vertex_output_world_position(inputs[1]);
        compiler->vertex_output_uv0(inputs[2]);
        compiler->vertex_output_uv1(inputs[3]);
        compiler->vertex_output_world_normal(inputs[4]);
        compiler->vertex_output_world_tangent(inputs[5]);
        compiler->vertex_output_world_bitangent(inputs[6]);
        compiler->vertex_output_color(inputs[7]);
        return 0;
    }

    bool VertexNode::is_removable() const
    {
        return false;
    }

    FragmentNode::FragmentNode()
    {
        inputs.push_back(new Color3InputPin(this, "Base Color"));
        inputs.push_back(new FloatInputPin(this, "Metalic"));
        inputs.push_back(new FloatInputPin(this, "Specular"));
        inputs.push_back(new FloatInputPin(this, "Roughness"));
        inputs.push_back(new Color3InputPin(this, "Emmisive"));
        inputs.push_back(new FloatInputPin(this, "Opacity", 1.f));
        inputs.push_back(new Vec3InputNoDefaultPin(this, "Position"));
        inputs.push_back(new Vec3InputNoDefaultPin(this, "Normal"));
    }

    const char* FragmentNode::name() const
    {
        return "Fragment";
    }

    size_t FragmentNode::compile(ShaderCompiler* compiler, MaterialOutputPin* pin)
    {
        compiler->fragment_output_base_color(inputs[0]);
        compiler->fragment_output_metalic(inputs[1]);
        compiler->fragment_output_specular(inputs[2]);
        compiler->fragment_output_roughness(inputs[3]);
        compiler->fragment_output_emissive(inputs[4]);
        compiler->fragment_output_opacity(inputs[5]);
        compiler->fragment_output_position(inputs[6]);
        compiler->fragment_output_normal(inputs[7]);
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
            inputs.push_back(new FloatInputPin(this, "In", 0.f));
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
            inputs.push_back(new FloatInputPin(this, "In", 0.f));
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
            inputs.push_back(new FloatInputPin(this, "In", 0.f));
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

    struct Dot : public MaterialNode {
        declare_material_node();
        Dot()
        {
            inputs.push_back(new Vec3InputPin(this, "A"));
            inputs.push_back(new Vec3InputPin(this, "B"));
            outputs.push_back(new FloatOutputNoDefaultPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->dot(inputs[0], inputs[1]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return MaterialNodeDataType::Vec3;
        }
    };


    struct Normalize : public MaterialNode {
        declare_material_node();
        Normalize()
        {
            inputs.push_back(new Vec3InputPin(this, "In"));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->normalize(inputs[0]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return inputs[0]->value_type();
        }
    };

    struct Pow : public MaterialNode {
        declare_material_node();

        Pow()
        {
            inputs.push_back(new FloatInputPin(this, "Value"));
            inputs.push_back(new FloatInputPin(this, "Base"));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->pow(inputs[0], inputs[1]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return inputs[0]->value_type();
        }
    };

    struct Floor : public MaterialNode {
        declare_material_node();

        Floor()
        {
            inputs.push_back(new FloatInputPin(this, "In"));
            outputs.push_back(new MaterialOutputPin(this, "Out"));
        }

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->floor(inputs[0]);
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return inputs[0]->value_type();
        }
    };

    implement_material_node(Sin, Math);
    implement_material_node(Cos, Math);
    implement_material_node(Tan, Math);
    implement_material_node(Dot, Math);
    implement_material_node(Normalize, Math);
    implement_material_node(Pow, Math);
    implement_material_node(Floor, Math);

    //////////////////////////// OPERATOR NODES ////////////////////////////

#define declare_type_operator(type, func_name)                                                                                   \
    struct CastTo##type : public MaterialNode {                                                                                  \
        declare_material_node();                                                                                                 \
        CastTo##type()                                                                                                           \
        {                                                                                                                        \
            inputs.push_back(new type##InputPin(this, "In"));                                                                    \
            outputs.push_back(new type##OutputNoDefaultPin(this, "Out"));                                                        \
        }                                                                                                                        \
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override                                                \
        {                                                                                                                        \
            return compiler->func_name##_op(inputs[0]);                                                                          \
        }                                                                                                                        \
        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override                                            \
        {                                                                                                                        \
            return MaterialNodeDataType::type;                                                                                   \
        }                                                                                                                        \
    };                                                                                                                           \
    implement_material_node(CastTo##type, Operators);

    declare_type_operator(Bool, bool);
    declare_type_operator(Int, int);
    declare_type_operator(UInt, uint);
    declare_type_operator(Float, float);
    declare_type_operator(BVec2, bvec2);
    declare_type_operator(BVec3, bvec3);
    declare_type_operator(BVec4, bvec4);
    declare_type_operator(IVec2, ivec2);
    declare_type_operator(IVec3, ivec3);
    declare_type_operator(IVec4, ivec4);
    declare_type_operator(UVec2, uvec2);
    declare_type_operator(UVec3, uvec3);
    declare_type_operator(UVec4, uvec4);
    declare_type_operator(Vec2, vec2);
    declare_type_operator(Vec3, vec3);
    declare_type_operator(Vec4, vec4);
    declare_type_operator(Color3, color3);
    declare_type_operator(Color4, color4);

    struct Add : public MaterialNode {
        declare_material_node();
        Add()
        {
            inputs.push_back(new FloatInputPin(this, "A", 0.f));
            inputs.push_back(new FloatInputPin(this, "B", 0.f));
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
            inputs.push_back(new FloatInputPin(this, "A", 0.f));
            inputs.push_back(new FloatInputPin(this, "B", 0.f));
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
            inputs.push_back(new FloatInputPin(this, "A", 0.f));
            inputs.push_back(new FloatInputPin(this, "B", 0.f));
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
            inputs.push_back(new FloatInputPin(this, "A", 0.f));
            inputs.push_back(new FloatInputPin(this, "B", 0.f));
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
    struct Construct2Components : public MaterialNode {

        Construct2Components()
        {
            inputs.push_back(new InputType(this, "X", typename InputType::NativeType(0)));
            inputs.push_back(new InputType(this, "Y", typename InputType::NativeType(0)));
            outputs.push_back(new OutputType(this, "Out"));
        }

        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override
        {
            return OutputType::data_type;
        }
    };

    template<typename InputType, typename OutputType>
    struct Construct3Components : public Construct2Components<InputType, OutputType> {
        Construct3Components()
        {
            Construct2Components<InputType, OutputType>::inputs.push_back(
                    new InputType(this, "Z", typename InputType::NativeType(0)));
        }
    };

    template<typename InputType, typename OutputType>
    struct Construct4Components : public Construct3Components<InputType, OutputType> {

        Construct4Components()
        {
            Construct3Components<InputType, OutputType>::inputs.push_back(
                    new InputType(this, "W", typename InputType::NativeType(1)));
        }
    };

    template<typename InputType>
    struct DecomposeVecBase : public MaterialNode {
        DecomposeVecBase()
        {
            inputs.push_back(new InputType(this, "Vec", typename InputType::NativeType(0)));
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

    struct ConstructVec2 : public Construct2Components<FloatInputPin, Vec2OutputNoDefaultPin> {
        declare_material_node();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->construct_vec2(inputs[0], inputs[1]);
        }
    };

    struct ConstructVec3 : public Construct3Components<FloatInputPin, Vec3OutputNoDefaultPin> {
        declare_material_node();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->construct_vec3(inputs[0], inputs[1], inputs[2]);
        }
    };

    struct ConstructVec4 : public Construct4Components<FloatInputPin, Vec4OutputNoDefaultPin> {
        declare_material_node();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->construct_vec4(inputs[0], inputs[1], inputs[2], inputs[3]);
        }
    };

    struct ConstructMat3 : public Construct3Components<Vec3InputPin, Mat3OutputNoDefaultPin> {
        declare_material_node();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->construct_mat3(inputs[0], inputs[1], inputs[2]);
        }
    };

    struct ConstructMat4 : public Construct4Components<Vec4InputPin, Mat4OutputNoDefaultPin> {
        declare_material_node();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override
        {
            return compiler->construct_mat4(inputs[0], inputs[1], inputs[2], inputs[3]);
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
    implement_material_node(ConstructMat3, Operators);
    implement_material_node(ConstructMat4, Operators);
    implement_material_node(DecomposeVec, Operators);


    //////////////////////////// GLOBALS NODES ////////////////////////////


#define declare_global_node(name, func, type)                                                                                    \
    struct name : public MaterialNode {                                                                                          \
        declare_material_node();                                                                                                 \
                                                                                                                                 \
        name()                                                                                                                   \
        {                                                                                                                        \
            outputs.push_back(new type##OutputNoDefaultPin(this, "Out"));                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override                                                \
        {                                                                                                                        \
            return compiler->func();                                                                                             \
        }                                                                                                                        \
                                                                                                                                 \
        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override                                            \
        {                                                                                                                        \
            return MaterialNodeDataType::type;                                                                                   \
        }                                                                                                                        \
    };                                                                                                                           \
    implement_material_node(name, Globals);

    declare_global_node(Projection, projection, Mat4);
    declare_global_node(View, view, Mat4);
    declare_global_node(ProjView, projview, Mat4);
    declare_global_node(InvProjView, inv_projview, Mat4);
    declare_global_node(ModelMatrix, model, Mat4);
    declare_global_node(CameraLocation, camera_location, Vec3);
    declare_global_node(Time, time, Float);
    declare_global_node(Gamma, gamma, Float);
    declare_global_node(DeltaTime, delta_time, Float);
    declare_global_node(FOV, fov, Float);
    declare_global_node(OrthoWidth, ortho_width, Float);
    declare_global_node(OrthoHeight, ortho_height, Float);
    declare_global_node(NearClipPlane, near_clip_plane, Float);
    declare_global_node(FarClipPlane, far_clip_plane, Float);
    declare_global_node(AspectRatio, aspect_ratio, Float);
    declare_global_node(CameraProjectionMode, camera_projection_mode, Int);
    declare_global_node(FragCoord, frag_coord, Vec2);
    declare_global_node(RenderTargetSize, render_target_size, Vec2);


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


    //////////////////////////// DYNAMIC PARAMETERS NODES ////////////////////////////

    void MaterialDynamicParameter::render()
    {
        MaterialNode::render();
        ImGuiRenderer::InputText("Name", node_name);
    }

    bool MaterialDynamicParameter::archive_process(Archive& ar)
    {
        if (!MaterialNode::archive_process(ar))
            return false;

        ar & node_name;
        return ar;
    }

#define declare_dynamic_node(name, func_name)                                                                                    \
    struct name##Parameter : public MaterialDynamicParameter {                                                                   \
        declare_material_node();                                                                                                 \
        name##Parameter()                                                                                                        \
        {                                                                                                                        \
            outputs.push_back(new name##OutputPin(this, "Out"));                                                                 \
        }                                                                                                                        \
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override                                                \
        {                                                                                                                        \
            return compiler->func_name##_parameter(node_name, pin->default_value());                                             \
        }                                                                                                                        \
        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override                                            \
        {                                                                                                                        \
            return MaterialNodeDataType::name;                                                                                   \
        }                                                                                                                        \
    };                                                                                                                           \
    implement_material_node(name##Parameter, Parameters);

    declare_dynamic_node(Bool, bool);
    declare_dynamic_node(Int, int);
    declare_dynamic_node(UInt, uint);
    declare_dynamic_node(Float, float);
    declare_dynamic_node(BVec2, bvec2);
    declare_dynamic_node(BVec3, bvec3);
    declare_dynamic_node(BVec4, bvec4);
    declare_dynamic_node(IVec2, ivec2);
    declare_dynamic_node(IVec3, ivec3);
    declare_dynamic_node(IVec4, ivec4);
    declare_dynamic_node(UVec2, uvec2);
    declare_dynamic_node(UVec3, uvec3);
    declare_dynamic_node(UVec4, uvec4);
    declare_dynamic_node(Vec2, vec2);
    declare_dynamic_node(Vec3, vec3);
    declare_dynamic_node(Vec4, vec4);
    declare_dynamic_node(Color3, color3);
    declare_dynamic_node(Color4, color4);

    //////////////////////////// TEXTURE NODES ////////////////////////////

#define declare_render_target_texture(name, func)                                                                                \
    struct name##Texture : public MaterialNode {                                                                                 \
        declare_material_node();                                                                                                 \
                                                                                                                                 \
        name##Texture()                                                                                                          \
        {                                                                                                                        \
            outputs.push_back(new Color4OutputNoDefaultPin(this, "Color"));                                                      \
            inputs.push_back(new MaterialInputPin(this, "Sampler"));                                                             \
            inputs.push_back(new MaterialInputPin(this, "UV"));                                                                  \
        }                                                                                                                        \
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override                                                \
        {                                                                                                                        \
            return compiler->func##_texture(inputs[0], inputs[1]);                                                               \
        }                                                                                                                        \
    };                                                                                                                           \
    implement_material_node(name##Texture, Texture);

    Sampler::Sampler()
    {
        outputs.push_back(new SamplerOutputNoDefaultPin(this, "Out"));
    }

    size_t Sampler::compile(ShaderCompiler* compiler, MaterialOutputPin* pin)
    {
        return compiler->sampler(sampler);
    }

    bool Sampler::archive_process(Archive& ar)
    {
        if (!MaterialNode::archive_process(ar))
            return false;

        return sampler.archive_process(ar, true);
    }

    Texture2D::Texture2D()
    {
        outputs.push_back(new Color4OutputNoDefaultPin(this, "Color"));
        inputs.push_back(new MaterialInputPin(this, "Sampler"));
        inputs.push_back(new MaterialInputPin(this, "UV"));
    }

    void Texture2D::render()
    {
        if (texture && texture->has_object())
        {
            Engine::Sampler* sampler                   = Icons::default_sampler();
            ImGuiRenderer::ImGuiTexture* imgui_texture = ImGuiRenderer::Window::current()->create_texture(texture, sampler);
            ImGui::Image(imgui_texture->handle(), {100, 100});
        }
    }

    bool Texture2D::archive_process(Archive& ar)
    {
        if (!MaterialNode::archive_process(ar))
            return false;

        return texture.archive_process(ar, true);
    }

    size_t Texture2D::compile(ShaderCompiler* compiler, MaterialOutputPin* pin)
    {
        return compiler->texture_2d(texture, inputs[0], inputs[1]);
    }

    implement_material_node(Sampler, Texture);
    implement_material_node(Texture2D, Texture);
    declare_render_target_texture(BaseColor, base_color);
    declare_render_target_texture(Position, position);
    declare_render_target_texture(Normal, normal);
    declare_render_target_texture(Emissive, emissive);
    declare_render_target_texture(DataBuffer, data_buffer);
    declare_render_target_texture(SceneOutput, scene_output);

    //////////////////////////// INPUT NODES ////////////////////////////

#define declare_input_attribute(class_name, func_name, type)                                                                     \
    struct class_name##Attribute : public MaterialNode {                                                                         \
        declare_material_node();                                                                                                 \
        byte index = 0;                                                                                                          \
                                                                                                                                 \
        class_name##Attribute()                                                                                                  \
        {                                                                                                                        \
            outputs.push_back(new type##OutputNoDefaultPin(this, "Out"));                                                        \
        }                                                                                                                        \
                                                                                                                                 \
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override                                                \
        {                                                                                                                        \
            return compiler->vertex_##func_name##_attribute(index);                                                              \
        }                                                                                                                        \
                                                                                                                                 \
        void render() override                                                                                                   \
        {                                                                                                                        \
            ImGui::InputScalar("Index", ImGuiDataType_U8, &index);                                                               \
        }                                                                                                                        \
                                                                                                                                 \
        bool archive_process(Archive& ar) override                                                                               \
        {                                                                                                                        \
            if (!MaterialNode::archive_process(ar))                                                                              \
                return false;                                                                                                    \
            return ar & index;                                                                                                   \
        }                                                                                                                        \
    };                                                                                                                           \
    implement_material_node(class_name##Attribute, Inputs);

    declare_input_attribute(Position, position, Vec3);
    declare_input_attribute(Normal, normal, Vec3);
    declare_input_attribute(TexCoord, uv, Vec2);
    declare_input_attribute(Binormal, binormal, Vec3);
    declare_input_attribute(Tangent, tangent, Vec3);
    declare_input_attribute(Color, color, Vec4);


}// namespace Engine::MaterialNodes
