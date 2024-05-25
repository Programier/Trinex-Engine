#include <Core/class.hpp>
#include <Core/engine_config.hpp>
#include <Core/enum.hpp>
#include <Core/file_manager.hpp>
#include <Core/group.hpp>
#include <Core/property.hpp>
#include <Graphics/visual_material.hpp>


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

    implement_struct(Name, Engine::VisualMaterial, );

    implement_engine_class(VisualMaterial, Class::IsAsset);
    implement_initialize_class(VisualMaterial)
    {
        Class* self       = This::static_class_instance();
        Enum* domain_enum = Enum::static_find("Engine::MaterialDomain", true);
        self->add_property(new EnumProperty("Domain", "Domain of this material", &This::domain, domain_enum));
    }

    VisualMaterial::Pin::Pin(Node* node, Name name) : m_node(node), m_name(name)
    {}

    VisualMaterial::Pin::~Pin()
    {}

    VisualMaterial::Node* VisualMaterial::Pin::node() const
    {
        return m_node;
    }

    Name VisualMaterial::Pin::name() const
    {
        return m_name;
    }

    bool VisualMaterial::Pin::has_links() const
    {
        return links_count() > 0;
    }

    VisualMaterial::InputPin::InputPin(Node* node, Name name) : Pin(node, name)
    {
        m_linked_to = nullptr;
    }

    VisualMaterial::PinType VisualMaterial::Pin::type() const
    {
        return PinType::Undefined;
    }

    void* VisualMaterial::Pin::default_value()
    {
        return nullptr;
    }


    VisualMaterial::PinKind VisualMaterial::InputPin::kind() const
    {
        return PinKind::Input;
    }

    size_t VisualMaterial::InputPin::links_count() const
    {
        return m_linked_to ? 1 : 0;
    }

    void VisualMaterial::InputPin::unlink()
    {
        if (m_linked_to)
        {
            m_linked_to->unlink_pin_internal(this);
            m_linked_to = nullptr;
        }
    }

    VisualMaterial::OutputPin* VisualMaterial::InputPin::linked_to() const
    {
        return m_linked_to;
    }

    void VisualMaterial::InputPin::create_link(OutputPin* pin)
    {
        unlink();
        pin->link_pin_internal(this);
        m_linked_to = pin;
    }


    VisualMaterial::OutputPin::OutputPin(Node* node, Name name) : Pin(node, name)
    {}


    VisualMaterial::PinKind VisualMaterial::OutputPin::kind() const
    {
        return PinKind::Output;
    }

    size_t VisualMaterial::OutputPin::links_count() const
    {
        return m_linked_to.size();
    }


    const Set<VisualMaterial::InputPin*>& VisualMaterial::OutputPin::linked_to() const
    {
        return m_linked_to;
    }

    void VisualMaterial::OutputPin::unlink()
    {
        Set<InputPin*> pins = std::move(m_linked_to);
        for (InputPin* pin : pins)
        {
            pin->unlink();
        }
    }

    void VisualMaterial::OutputPin::link_pin_internal(InputPin* pin)
    {
        pin->m_linked_to = this;
        m_linked_to.insert(pin);
    }

    void VisualMaterial::OutputPin::unlink_pin_internal(InputPin* pin)
    {
        m_linked_to.erase(pin);
    }

    VisualMaterial::Node& VisualMaterial::Node::init(VisualMaterial* material)
    {
        m_material = material;
        return *this;
    }

    bool VisualMaterial::Node::is_destroyable() const
    {
        return true;
    }

    Vector4D VisualMaterial::Node::header_color() const
    {
        return Vector4D(1.0, 0.0, 0.0, 1.0);
    }

    VisualMaterial::Expression VisualMaterial::Node::compile(OutputPin* pin, CompilerState& state)
    {
        return Expression("", PinType::Undefined);
    }

    static const char* slang_type_name(VisualMaterial::PinType type)
    {
        switch (type)
        {
            case VisualMaterial::PinType::Bool:
                return "bool";
            case VisualMaterial::PinType::Int:
                return "int";
            case VisualMaterial::PinType::UInt:
                return "uint";
            case VisualMaterial::PinType::Float:
                return "float";
            case VisualMaterial::PinType::BVec2:
                return "bool2";
            case VisualMaterial::PinType::BVec3:
                return "bool3";
            case VisualMaterial::PinType::BVec4:
                return "bool4";
            case VisualMaterial::PinType::IVec2:
                return "int2";
            case VisualMaterial::PinType::IVec3:
                return "int3";
            case VisualMaterial::PinType::IVec4:
                return "int4";
            case VisualMaterial::PinType::UVec2:
                return "uint2";
            case VisualMaterial::PinType::UVec3:
                return "uint3";
            case VisualMaterial::PinType::UVec4:
                return "uint4";
            case VisualMaterial::PinType::Vec2:
                return "float2";
            case VisualMaterial::PinType::Vec3:
            case VisualMaterial::PinType::Color3:
                return "float3";
            case VisualMaterial::PinType::Vec4:
            case VisualMaterial::PinType::Color4:
                return "float4";
            case VisualMaterial::PinType::Mat3:
                return "float3x3";
            case VisualMaterial::PinType::Mat4:
                return "float4x4";
            case VisualMaterial::PinType::Sampler:
                return "Sampler";
            case VisualMaterial::PinType::CombinedImageSampler2D:
                return "Sampler2D";
            case VisualMaterial::PinType::Texture2D:
                return "Texture2D";
            default:
                break;
        }
        return "Undefined";
    }

    static String create_default_value(VisualMaterial::PinType type, const void* data)
    {
        union Storage
        {
            const void* ptr;
            const bool* bool_ptr;
            const int_t* int_ptr;
            const uint_t* uint_ptr;
            const float* float_ptr;

            Storage(const void* ptr) : ptr(ptr)
            {}
        } storage(data);

        switch (type)
        {
            case VisualMaterial::PinType::Bool:
                return storage.bool_ptr && storage.bool_ptr[0] ? "true" : "false";
            case VisualMaterial::PinType::Int:
                return storage.ptr ? Strings::format("{}", storage.int_ptr[0]) : "0";
            case VisualMaterial::PinType::UInt:
                return storage.ptr ? Strings::format("{}", storage.uint_ptr[0]) : "0";
            case VisualMaterial::PinType::Float:
                return storage.ptr ? Strings::format("{:.6f}f", storage.float_ptr[0]) : "0.f";
            case VisualMaterial::PinType::BVec2:
                break;
            case VisualMaterial::PinType::BVec3:
                break;
            case VisualMaterial::PinType::BVec4:
                break;
            case VisualMaterial::PinType::IVec2:
                break;
            case VisualMaterial::PinType::IVec3:
                break;
            case VisualMaterial::PinType::IVec4:
                break;
            case VisualMaterial::PinType::UVec2:
                break;
            case VisualMaterial::PinType::UVec3:
                break;
            case VisualMaterial::PinType::UVec4:
                break;

            case VisualMaterial::PinType::Vec2:
                return storage.float_ptr ? Strings::format("float2({:.6f}, {:.6f})", storage.float_ptr[0], storage.float_ptr[1])
                                         : "float2(0.f, 0.f)";
            case VisualMaterial::PinType::Vec3:
            case VisualMaterial::PinType::Color3:
                return storage.float_ptr ? Strings::format("float3({:.6f}, {:.6f}, {:.6f})", storage.float_ptr[0],
                                                           storage.float_ptr[1], storage.float_ptr[2])
                                         : "float3(0.f, 0.f, 0.f)";
            case VisualMaterial::PinType::Vec4:
            case VisualMaterial::PinType::Color4:
                return storage.float_ptr ? Strings::format("float4({:.6f}, {:.6f}, {:.6f}, {:.6f})", storage.float_ptr[0],
                                                           storage.float_ptr[1], storage.float_ptr[2], storage.float_ptr[3])
                                         : "float4(0.f, 0.f, 0.f, 0.f)";

            default:
                return "";
        }

        return "";
    }

    VisualMaterial::Expression VisualMaterial::Node::compile(InputPin* pin, CompilerState& state)
    {
        if (OutputPin* out = pin->linked_to())
        {
            return state.pin_source(out);
        }

        if (pin->default_value())
        {
            return Expression(create_default_value(pin->type(), pin->default_value()), pin->type());
        }
        return Expression();
    }

    VisualMaterial* VisualMaterial::Node::material() const
    {
        return m_material;
    }

    const Vector<VisualMaterial::InputPin*>& VisualMaterial::Node::inputs() const
    {
        return m_inputs;
    }

    const Vector<VisualMaterial::OutputPin*>& VisualMaterial::Node::outputs() const
    {
        return m_outputs;
    }

    VisualMaterial::Node::~Node()
    {
        for (auto& input : m_inputs)
        {
            delete input;
        }

        for (auto& output : m_outputs)
        {
            delete output;
        }

        auto& nodes = m_material->m_nodes;
        for (size_t i = 0, count = m_material->m_nodes.size(); i < count; i++)
        {
            if (nodes[i] == this)
            {
                nodes.erase(nodes.begin() + i);
                break;
            }
        }
    }

    const char* VisualMaterial::Node::name() const
    {
        return "Undefined";
    }

    VisualMaterial::VisualMaterial() : domain(MaterialDomain::Surface)
    {
        create_node(Struct::static_find("Engine::VisualMaterial::Root", true));
    }

    VisualMaterial::~VisualMaterial()
    {
        Vector<Node*> nodes = std::move(m_nodes);

        for (Node* node : nodes)
        {
            delete node;
        }
    }

    const Vector<VisualMaterial::Node*>& VisualMaterial::nodes() const
    {
        return m_nodes;
    }

    VisualMaterial::Node* VisualMaterial::create_node(class Struct* node_struct)
    {
        Node* node = reinterpret_cast<Node*>(node_struct->create_struct());
        m_nodes.push_back(node);
        node->init(this);
        return node;
    }

    static String read_material_template(MaterialDomain domain)
    {
        static Enum* domain_enum = Enum::static_find("Engine::MaterialDomain", true);
        Name name                = domain_enum->entry(static_cast<EnumerateType>(domain))->name;

        Path file_path = engine_config.shaders_dir / "material_templates" / name.c_str() + ".slang";
        FileReader reader(file_path);

        if (reader.is_open())
        {
            return reader.read_string();
        }
        return "";
    }


    static bool compile_vertex_shader(String& template_source, size_t position, VisualMaterial::Node* root, MaterialDomain domain)
    {
        VisualMaterial::CompilerState compiler;


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

    static bool compile_fragment_shader(String& template_source, size_t position, VisualMaterial::Node* root,
                                        MaterialDomain domain)
    {
        VisualMaterial::CompilerState compiler;


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


    VisualMaterial::Expression VisualMaterial::CompilerState::create_variable(const Expression& in_expression)
    {
        if (in_expression.is_variable)
            return in_expression;

        String var_name = Strings::format("var_{}", locals.size());
        locals.push_back(Strings::format("{} {} = {};", slang_type_name(in_expression.type), var_name, in_expression.code));
        return Expression(var_name, in_expression.type, true);
    }

    VisualMaterial::Expression VisualMaterial::CompilerState::expression_cast(const Expression& in_expression,
                                                                              VisualMaterial::PinType out_type)
    {
        if (!VisualMaterial::is_convertable(in_expression.type, out_type))
            return Expression("", PinType::Undefined);

        if (in_expression.type != out_type)
        {
            if (VisualMaterial::is_scalar(in_expression.type))
            {
                if (VisualMaterial::is_scalar(out_type))
                {
                    return Expression(Strings::format("{}({})", slang_type_name(out_type), in_expression.code.c_str()), out_type);
                }

                if (VisualMaterial::is_vector(out_type))
                {
                    Expression variable = create_variable(in_expression);
                    uint_t components   = VisualMaterial::components_count(out_type);

                    if (components == 2)
                        return Expression(Strings::format("{}({}, {})", slang_type_name(out_type), variable.code, variable.code),
                                          out_type);
                    if (components == 3)
                        return Expression(Strings::format("{}({}, {}, {})", slang_type_name(out_type), variable.code,
                                                          variable.code, variable.code),
                                          out_type);
                    if (components == 4)
                        return Expression(Strings::format("{}({}, {}, {}, {})", slang_type_name(out_type), variable.code,
                                                          variable.code, variable.code, variable.code),
                                          out_type);
                }
            }
            else if (VisualMaterial::is_vector(in_expression.type))
            {
                if (VisualMaterial::is_scalar(out_type))
                {
                    return Expression(Strings::format("{}({}.x)", slang_type_name(out_type), in_expression.code.c_str()),
                                      out_type);
                }

                if (VisualMaterial::is_vector(out_type))
                {
                    Expression variable = create_variable(in_expression);

                    uint_t in_components  = VisualMaterial::components_count(in_expression.type);
                    uint_t out_components = VisualMaterial::components_count(out_type);

                    PinType in_component_type  = VisualMaterial::components_type(in_expression.type);
                    PinType out_component_type = VisualMaterial::components_type(out_type);


                    if (in_component_type == out_component_type)
                    {
                        static const char* components_mask[] = {"xy", "xyz", "xyzw"};
                        if (in_components > out_components)
                        {
                            return Expression(Strings::format("{}({}.{})", slang_type_name(out_type), variable.code.c_str(),
                                                              components_mask[out_components - 2]),
                                              out_type);
                        }

                        uint_t diff          = out_components - in_components;
                        String default_value = create_default_value(out_component_type, nullptr);

                        if (diff == 1)
                        {
                            return Expression(Strings::format("{}({}.{}, {})", slang_type_name(out_type), variable.code.c_str(),
                                                              components_mask[in_components - 2], default_value),
                                              out_type);
                        }
                        else if (diff == 2)
                        {
                            return Expression(Strings::format("{}({}.{}, {}, {})", slang_type_name(out_type),
                                                              variable.code.c_str(), components_mask[in_components - 2],
                                                              default_value, default_value),
                                              out_type);
                        }
                    }

                    String default_value = create_default_value(out_component_type, nullptr);
                    String code          = slang_type_name(out_type);
                    code.push_back('(');

                    for (uint_t i = 0; i < out_components; ++i)
                    {
                        static constexpr const char* components_mask[] = {"x", "y", "z", "w"};

                        if (i < in_components)
                        {
                            Expression component = expression_cast(
                                    Expression(Strings::format("{}.{}", variable.code, components_mask[i]), in_component_type),
                                    out_component_type);
                            code += component.code;
                        }
                        else
                        {
                            code += default_value;
                        }

                        code += i == out_components - 1 ? ")" : ", ";
                    }

                    return Expression(code, out_type, false);
                }
            }
            else if (VisualMaterial::is_matrix(in_expression.type) && VisualMaterial::is_matrix(out_type))
            {
                uint_t in_components  = VisualMaterial::components_count(in_expression.type);
                uint_t out_components = VisualMaterial::components_count(out_type);

                if (out_components < in_components)
                {
                    return Expression(Strings::format("{}({})", slang_type_name(out_type), in_expression.code), out_type);
                }
                else
                {
                    throw EngineException("Unimplemented matrix cast");
                }
            }
        }
        else
        {
            return in_expression;
        }

        return Expression("", PinType::Undefined);
    }

    String VisualMaterial::CompilerState::create_header(const char* prefix) const
    {
        String result = {};

        for (auto& line : locals)
        {
            result += Strings::format("{}{}\n", prefix, line);
        }
        return result;
    }

    VisualMaterial::Expression VisualMaterial::CompilerState::pin_source(OutputPin* pin)
    {
        Expression& expression = m_internal_cache[pin->id()];

        if (!expression.is_valid())
        {
            expression = pin->node()->compile(pin, *this);
            if (pin->links_count() > 1)
            {
                expression = create_variable(expression);
            }
        }
        return expression;
    }

    VisualMaterial::Expression VisualMaterial::CompilerState::pin_source(InputPin* pin)
    {
        return pin->node()->compile(pin, *this);
    }

    namespace Nodes
    {
        using OutputPin     = VisualMaterial::OutputPin;
        using Expression    = VisualMaterial::Expression;
        using CompilerState = VisualMaterial::CompilerState;
        using PinType       = VisualMaterial::PinType;

#define implement_material_node(node_name, group_name)                                                                           \
    implement_struct(node_name, Engine::VisualMaterial, Engine::VisualMaterial::Node).push([]() {                                \
        Struct::static_find(MAKE_ENTITY_FULL_NAME(node_name, Engine::VisualMaterial), true)                                      \
                ->struct_constructor(static_void_constructor_of<node_name>)                                                      \
                .group(Group::find("Engine::VisualMaterialNodes::" #group_name, true));                                          \
    });

        struct Root : public VisualMaterial::Node {
            Root()
            {
                m_inputs.push_back(new VisualMaterial::Color3InputPin(this, "Base Color"));
                m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "Opacity"));
                m_inputs.push_back(new VisualMaterial::Color3InputPin(this, "Emissive"));
                m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "Specular"));
                m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "Metalness"));
                m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "Roughness"));
                m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "AO"));
                m_inputs.push_back(new VisualMaterial::Vec3InputPinND(this, "Normal"));
                m_inputs.push_back(new VisualMaterial::Vec3InputPin(this, "Position Offset"));
            }

            const char* name() const override
            {
                return "Material";
            }

            bool is_destroyable() const override
            {
                return false;
            }

            VisualMaterial::Expression compile(VisualMaterial::InputPin* pin, CompilerState& state) override
            {
                auto expression = Node::compile(pin, state);
                return state.expression_cast(expression, pin->type());
            }
        };

        implement_struct(Root, Engine::VisualMaterial, Engine::VisualMaterial::Node).push([]() {
            Struct::static_find("Engine::VisualMaterial::Root", true)->struct_constructor(static_void_constructor_of<Root>);
        });

        //////////////////////// CONSTANTS NODES ////////////////////////

#define implement_constant_node(type)                                                                                            \
    struct type : public VisualMaterial::Node {                                                                                  \
        type()                                                                                                                   \
        {                                                                                                                        \
            m_outputs.push_back(new VisualMaterial::type##OutputPin(this, "Value"));                                             \
        }                                                                                                                        \
        const char* name() const override                                                                                        \
        {                                                                                                                        \
            return #type;                                                                                                        \
        }                                                                                                                        \
        Expression compile(OutputPin* pin, CompilerState& state) override                                                        \
        {                                                                                                                        \
            return Expression(create_default_value(PinType::type, pin->default_value()), PinType::type);                         \
        }                                                                                                                        \
    };                                                                                                                           \
    implement_material_node(type, Constants)


        implement_constant_node(Bool);
        implement_constant_node(Int);
        implement_constant_node(UInt);
        implement_constant_node(Float);
        implement_constant_node(BVec2);
        implement_constant_node(BVec3);
        implement_constant_node(BVec4);
        implement_constant_node(IVec2);
        implement_constant_node(IVec3);
        implement_constant_node(IVec4);
        implement_constant_node(UVec2);
        implement_constant_node(UVec3);
        implement_constant_node(UVec4);
        implement_constant_node(Vec2);
        implement_constant_node(Vec3);
        implement_constant_node(Vec4);
        implement_constant_node(Color3);
        implement_constant_node(Color4);

        //////////////////////// CAST NODES ////////////////////////


#define implement_cast_node(type)                                                                                                \
    struct To##type : public VisualMaterial::Node {                                                                              \
        To##type()                                                                                                               \
        {                                                                                                                        \
            m_inputs.push_back(new VisualMaterial::type##InputPin(this, "In"));                                                  \
            m_outputs.push_back(new VisualMaterial::type##OutputPinND(this, "Out"));                                             \
        }                                                                                                                        \
        const char* name() const override                                                                                        \
        {                                                                                                                        \
            return "To" #type;                                                                                                   \
        }                                                                                                                        \
        Expression compile(OutputPin* pin, CompilerState& state) override                                                        \
        {                                                                                                                        \
            return state.expression_cast(state.pin_source(m_inputs[0]), PinType::type);                                          \
        }                                                                                                                        \
    };                                                                                                                           \
    implement_material_node(To##type, Casts)

        implement_cast_node(Bool);
        implement_cast_node(Int);
        implement_cast_node(UInt);
        implement_cast_node(Float);
        implement_cast_node(BVec2);
        implement_cast_node(BVec3);
        implement_cast_node(BVec4);
        implement_cast_node(IVec2);
        implement_cast_node(IVec3);
        implement_cast_node(IVec4);
        implement_cast_node(UVec2);
        implement_cast_node(UVec3);
        implement_cast_node(UVec4);
        implement_cast_node(Vec2);
        implement_cast_node(Vec3);
        implement_cast_node(Vec4);
        implement_cast_node(Color3);
        implement_cast_node(Color4);

        //////////////////////// MATH NODES ////////////////////////

        struct Abs : public VisualMaterial::Node {
            Abs()
            {
                m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "In"));
                m_outputs.push_back(new VisualMaterial::FloatOutputPinND(this, "Out"));
            }

            const char* name() const override
            {
                return "Abs";
            }

            Expression compile(OutputPin* pin, CompilerState& state) override
            {
                Expression expression  = state.pin_source(m_inputs[0]);
                expression             = state.expression_cast(expression, VisualMaterial::to_int_or_float(expression.type));
                expression.code        = Strings::format("abs({})", expression.code);
                expression.is_variable = false;
                return expression;
            }
        };

        struct Sin : public VisualMaterial::Node {
            Sin()
            {
                m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "In"));
                m_outputs.push_back(new VisualMaterial::FloatOutputPinND(this, "Out"));
            }

            const char* name() const override
            {
                return "Sin";
            }

            Expression compile(OutputPin* pin, CompilerState& state) override
            {
                Expression expression  = state.pin_source(m_inputs[0]);
                expression             = state.expression_cast(expression, VisualMaterial::to_floating_point(expression.type));
                expression.code        = Strings::format("sin({})", expression.code);
                expression.is_variable = false;
                return expression;
            }
        };

        struct Cos : public VisualMaterial::Node {
            Cos()
            {
                m_inputs.push_back(new VisualMaterial::FloatInputPin(this, "In"));
                m_outputs.push_back(new VisualMaterial::FloatOutputPinND(this, "Out"));
            }

            const char* name() const override
            {
                return "Cos";
            }

            Expression compile(OutputPin* pin, CompilerState& state) override
            {

                Expression expression  = state.pin_source(m_inputs[0]);
                expression             = state.expression_cast(expression, VisualMaterial::to_floating_point(expression.type));
                expression.code        = Strings::format("cos({})", expression.code);
                expression.is_variable = false;
                return expression;
            }
        };

        implement_material_node(Sin, Math);
        implement_material_node(Cos, Math);
    }// namespace Nodes

}// namespace Engine
