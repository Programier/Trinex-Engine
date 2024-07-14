#include <Core/class.hpp>
#include <Core/enum.hpp>
#include <Core/file_manager.hpp>
#include <Core/group.hpp>
#include <Core/property.hpp>
#include <Engine/project.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>

namespace Engine
{
    static inline constexpr const char* vertex_material_source_attribute   = "@1";
    static inline constexpr const char* fragment_material_source_attribute = "@2";

    static inline constexpr size_t base_color_index      = 0;
    static inline constexpr size_t opacity_index         = 1;
    static inline constexpr size_t emissive_index        = 2;
    static inline constexpr size_t specular_index        = 3;
    static inline constexpr size_t metalness_index       = 4;
    static inline constexpr size_t roughness_index       = 5;
    static inline constexpr size_t ao_index              = 6;
    static inline constexpr size_t normal_index          = 7;
    static inline constexpr size_t position_offset_index = 8;

    implement_engine_class(VisualMaterial, Class::IsAsset)
    {
        Class* self       = This::static_class_instance();
        Enum* domain_enum = Enum::static_find("Engine::MaterialDomain", true);
        self->add_property(new EnumProperty("Domain", "Domain of this material", &This::domain, domain_enum));
    }


    VisualMaterial::VisualMaterial() : domain(MaterialDomain::Surface)
    {
        create_node(Class::static_find("Engine::VisualMaterialGraph::Root", true));
    }

    const Vector<Pointer<VisualMaterialGraph::Node>>& VisualMaterial::nodes() const
    {
        return m_nodes;
    }

    VisualMaterialGraph::Node* VisualMaterial::create_node(class Class* node_class)
    {
        if (node_class->is_a<VisualMaterialGraph::Node>())
        {
            VisualMaterialGraph::Node* node = Object::instance_cast<VisualMaterialGraph::Node>(node_class->create_object());
            if (node)
                m_nodes.push_back(node);
            return node;
        }
        return nullptr;
    }

    static String read_material_template(MaterialDomain domain)
    {
        static Enum* domain_enum = Enum::static_find("Engine::MaterialDomain", true);
        Name name                = domain_enum->entry(static_cast<EnumerateType>(domain))->name;

        Path file_path = Path(Project::shaders_dir) / "material_templates" / name.c_str() + ".slang";
        FileReader reader(file_path);

        if (reader.is_open())
        {
            return reader.read_string();
        }
        return "";
    }


    static bool compile_vertex_shader(String& template_source, size_t position, VisualMaterialGraph::Node* root,
                                      MaterialDomain domain)
    {
        VisualMaterialGraph::CompilerState compiler;


        static constexpr const char* format = "Material material = (Material)0;\n\n"
                                              "{0}\n"
                                              "\tmaterial.world_normal      = {1};\n"
                                              "\tmaterial.position_offset   = {2};\n"
                                              "\treturn material;";

        auto normal = root->compile(root->inputs()[normal_index], compiler);

        if (!normal.is_valid())
        {
            normal.code = "vertex_factory.get_world_normal()";
        }

        auto position_offset = root->compile(root->inputs()[position_offset_index], compiler);
        if (!position_offset.is_valid())
            return false;

        String header     = compiler.create_header("\t");
        String out_source = Strings::format(format, header, normal.code, position_offset.code);
        template_source.replace(position, std::strlen(vertex_material_source_attribute), out_source);
        return true;
    }

    static bool compile_fragment_shader(String& template_source, size_t position, VisualMaterialGraph::Node* root,
                                        MaterialDomain domain)
    {
        VisualMaterialGraph::CompilerState compiler;


        static constexpr const char* format = "Material material;\n\n"
                                              "{0}\n"
                                              "\tmaterial.base_color        = float3(0.f, 0.f, 0.f);\n"
                                              "\tmaterial.emissive          = float3(0.f, 0.f, 0.f);\n"
                                              "\tmaterial.world_normal      = {1};\n"
                                              "\tmaterial.position_offset   = {2};\n"
                                              "\tmaterial.specular = 0.f;\n"
                                              "\tmaterial.metalness = 0.f;\n"
                                              "\tmaterial.roughness = 0.f;\n"
                                              "\tmaterial.opacity = 0.f;\n"
                                              "\tmaterial.AO = 1.f;\n"
                                              "\treturn material;";

        auto normal = root->compile(root->inputs()[normal_index], compiler);

        if (!normal.is_valid())
        {
            normal.code = "vertex_factory.get_world_normal()";
        }

        auto position_offset = root->compile(root->inputs()[position_offset_index], compiler);
        if (!position_offset.is_valid())
            return false;

        String header     = compiler.create_header("\t");
        String out_source = Strings::format(format, header, normal.code, position_offset.code);
        template_source.replace(position, std::strlen(vertex_material_source_attribute), out_source);
        return true;
    }

    bool VisualMaterial::shader_source(String& out_source)
    {
        String template_source = read_material_template(domain);
        bool status            = true;

        // Compile vertex shader
        {
            auto pos = template_source.find(vertex_material_source_attribute);
            if (pos != String::npos)
            {
                status = compile_vertex_shader(template_source, pos, nodes()[0], domain);
            }
        }

        // Compile fragment shader
        if (status)
        {
            auto pos = template_source.find(fragment_material_source_attribute);
            if (pos != String::npos)
            {
                status = compile_fragment_shader(template_source, pos, nodes()[0], domain);
            }
        }

        if (status)
        {
            out_source = std::move(template_source);
        }

        return status;
    }

    namespace Nodes
    {


        //////////////////////// CAST NODES ////////////////////////


// #define implement_cast_node(type)                                                                                                \
//     struct To##type : public VisualMaterial::Node {                                                                              \
//         To##type()                                                                                                               \
//         {                                                                                                                        \
//             m_inputs.push_back(new VisualMaterial::type##InputPin(this, "In"));                                                  \
//             m_outputs.push_back(new VisualMaterial::type##OutputPinND(this, "Out"));                                             \
//         }                                                                                                                        \
//         const char* name() const override                                                                                        \
//         {                                                                                                                        \
//             return "To" #type;                                                                                                   \
//         }                                                                                                                        \
//         Expression compile(OutputPin* pin, CompilerState& state) override                                                        \
//         {                                                                                                                        \
//             return state.expression_cast(state.pin_source(m_inputs[0]), PinType::type);                                          \
//         }                                                                                                                        \
//     };                                                                                                                           \
//     implement_material_node(To##type, Casts)

//         implement_cast_node(Bool);
//         implement_cast_node(Int);
//         implement_cast_node(UInt);
//         implement_cast_node(Float);
//         implement_cast_node(BVec2);
//         implement_cast_node(BVec3);
//         implement_cast_node(BVec4);
//         implement_cast_node(IVec2);
//         implement_cast_node(IVec3);
//         implement_cast_node(IVec4);
//         implement_cast_node(UVec2);
//         implement_cast_node(UVec3);
//         implement_cast_node(UVec4);
//         implement_cast_node(Vec2);
//         implement_cast_node(Vec3);
//         implement_cast_node(Vec4);
//         implement_cast_node(Color3);
//         implement_cast_node(Color4);

//         //////////////////////// MATH NODES ////////////////////////

//         struct Abs : public VisualMaterial::Node {
//             Abs()
//             {
//                 m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "In"));
//                 m_outputs.push_back(new VisualMaterial::FloatOutputPinND(this, "Out"));
//             }

//             const char* name() const override
//             {
//                 return "Abs";
//             }

//             Expression compile(OutputPin* pin, CompilerState& state) override
//             {
//                 Expression expression  = state.pin_source(m_inputs[0]);
//                 expression             = state.expression_cast(expression, VisualMaterial::to_int_or_float(expression.type));
//                 expression.code        = Strings::format("abs({})", expression.code);
//                 expression.is_variable = false;
//                 return expression;
//             }
//         };

//         struct Sin : public VisualMaterial::Node {
//             Sin()
//             {
//                 m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "In"));
//                 m_outputs.push_back(new VisualMaterial::FloatOutputPinND(this, "Out"));
//             }

//             const char* name() const override
//             {
//                 return "Sin";
//             }

//             Expression compile(OutputPin* pin, CompilerState& state) override
//             {
//                 Expression expression  = state.pin_source(m_inputs[0]);
//                 expression             = state.expression_cast(expression, VisualMaterial::to_floating_point(expression.type));
//                 expression.code        = Strings::format("sin({})", expression.code);
//                 expression.is_variable = false;
//                 return expression;
//             }
//         };

//         struct Cos : public VisualMaterial::Node {
//             Cos()
//             {
//                 m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "In"));
//                 m_outputs.push_back(new VisualMaterial::FloatOutputPinND(this, "Out"));
//             }

//             const char* name() const override
//             {
//                 return "Cos";
//             }

//             Expression compile(OutputPin* pin, CompilerState& state) override
//             {

//                 Expression expression  = state.pin_source(m_inputs[0]);
//                 expression             = state.expression_cast(expression, VisualMaterial::to_floating_point(expression.type));
//                 expression.code        = Strings::format("cos({})", expression.code);
//                 expression.is_variable = false;
//                 return expression;
//             }
//         };

//         implement_material_node(Sin, Math);
//         implement_material_node(Cos, Math);
    }// namespace Nodes

}// namespace Engine
