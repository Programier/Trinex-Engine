#include <Core/class.hpp>
#include <Core/group.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>

namespace Engine::VisualMaterialGraph
{
    implement_class_default_init(Engine::VisualMaterialGraph, Node, 0);

    Pin::Pin(Node* node, Name name) : m_node(node), m_name(name)
    {}

    Pin::~Pin()
    {}

    Node* Pin::node() const
    {
        return m_node;
    }

    Name Pin::name() const
    {
        return m_name;
    }

    bool Pin::has_links() const
    {
        return links_count() > 0;
    }

    InputPin::InputPin(Node* node, Name name) : Pin(node, name)
    {
        m_linked_to = nullptr;
    }

    PinType Pin::type() const
    {
        return PinType::Undefined;
    }

    void* Pin::default_value()
    {
        return nullptr;
    }


    PinKind InputPin::kind() const
    {
        return PinKind::Input;
    }

    size_t InputPin::links_count() const
    {
        return m_linked_to ? 1 : 0;
    }

    void InputPin::unlink()
    {
        if (m_linked_to)
        {
            m_linked_to->unlink_pin_internal(this);
            m_linked_to = nullptr;
        }
    }

    OutputPin* InputPin::linked_to() const
    {
        return m_linked_to;
    }

    void InputPin::create_link(OutputPin* pin)
    {
        unlink();
        pin->link_pin_internal(this);
        m_linked_to = pin;
    }

    OutputPin::OutputPin(Node* node, Name name) : Pin(node, name)
    {}

    PinKind OutputPin::kind() const
    {
        return PinKind::Output;
    }

    size_t OutputPin::links_count() const
    {
        return m_linked_to.size();
    }


    const Set<InputPin*>& OutputPin::linked_to() const
    {
        return m_linked_to;
    }

    void OutputPin::unlink()
    {
        Set<InputPin*> pins = std::move(m_linked_to);
        for (InputPin* pin : pins)
        {
            pin->unlink();
        }
    }

    void OutputPin::link_pin_internal(InputPin* pin)
    {
        pin->m_linked_to = this;
        m_linked_to.insert(pin);
    }

    void OutputPin::unlink_pin_internal(InputPin* pin)
    {
        m_linked_to.erase(pin);
    }

    bool Node::is_destroyable() const
    {
        return true;
    }

    Vector4D Node::header_color() const
    {
        return Vector4D(1.0, 0.0, 0.0, 1.0);
    }

    Expression Node::compile(OutputPin* pin, CompilerState& state)
    {
        return Expression("", PinType::Undefined);
    }

    Expression Node::compile(InputPin* pin, CompilerState& state)
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

    const Vector<InputPin*>& Node::inputs() const
    {
        return m_inputs;
    }

    const Vector<OutputPin*>& Node::outputs() const
    {
        return m_outputs;
    }

    Node::~Node()
    {
        for (auto& input : m_inputs)
        {
            delete input;
        }

        for (auto& output : m_outputs)
        {
            delete output;
        }
    }

    const char* Node::name() const
    {
        return "Undefined";
    }

    Expression CompilerState::create_variable(const Expression& in_expression)
    {
        if (in_expression.is_variable)
            return in_expression;

        String var_name = Strings::format("var_{}", locals.size());
        locals.push_back(Strings::format("{} {} = {};", slang_type_name(in_expression.type), var_name, in_expression.code));
        return Expression(var_name, in_expression.type, true);
    }

    Expression CompilerState::expression_cast(const Expression& in_expression, PinType out_type)
    {
        if (!is_convertable(in_expression.type, out_type))
            return Expression("", PinType::Undefined);

        if (in_expression.type != out_type)
        {
            if (is_scalar(in_expression.type))
            {
                if (is_scalar(out_type))
                {
                    return Expression(Strings::format("{}({})", slang_type_name(out_type), in_expression.code.c_str()), out_type);
                }

                if (is_vector(out_type))
                {
                    Expression variable = create_variable(in_expression);
                    uint_t components   = components_count(out_type);

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
            else if (is_vector(in_expression.type))
            {
                if (is_scalar(out_type))
                {
                    return Expression(Strings::format("{}({}.x)", slang_type_name(out_type), in_expression.code.c_str()),
                                      out_type);
                }

                if (is_vector(out_type))
                {
                    Expression variable = create_variable(in_expression);

                    uint_t in_components  = components_count(in_expression.type);
                    uint_t out_components = components_count(out_type);

                    PinType in_component_type  = components_type(in_expression.type);
                    PinType out_component_type = components_type(out_type);


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
            else if (is_matrix(in_expression.type) && is_matrix(out_type))
            {
                uint_t in_components  = components_count(in_expression.type);
                uint_t out_components = components_count(out_type);

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

    String CompilerState::create_header(const char* prefix) const
    {
        String result = {};

        for (auto& line : locals)
        {
            result += Strings::format("{}{}\n", prefix, line);
        }
        return result;
    }

    Expression CompilerState::pin_source(OutputPin* pin)
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

    Expression CompilerState::pin_source(InputPin* pin)
    {
        return pin->node()->compile(pin, *this);
    }

    ENGINE_EXPORT const char* slang_type_name(PinType type)
    {

        switch (type)
        {
            case PinType::Bool:
                return "bool";
            case PinType::Int:
                return "int";
            case PinType::UInt:
                return "uint";
            case PinType::Float:
                return "float";
            case PinType::BVec2:
                return "bool2";
            case PinType::BVec3:
                return "bool3";
            case PinType::BVec4:
                return "bool4";
            case PinType::IVec2:
                return "int2";
            case PinType::IVec3:
                return "int3";
            case PinType::IVec4:
                return "int4";
            case PinType::UVec2:
                return "uint2";
            case PinType::UVec3:
                return "uint3";
            case PinType::UVec4:
                return "uint4";
            case PinType::Vec2:
                return "float2";
            case PinType::Vec3:
            case PinType::Color3:
                return "float3";
            case PinType::Vec4:
            case PinType::Color4:
                return "float4";
            case PinType::Mat3:
                return "float3x3";
            case PinType::Mat4:
                return "float4x4";
            case PinType::Sampler:
                return "Sampler";
            case PinType::CombinedImageSampler2D:
                return "Sampler2D";
            case PinType::Texture2D:
                return "Texture2D";
            default:
                break;
        }
        return "Undefined";
    }


    ENGINE_EXPORT String create_default_value(PinType type, const void* data)
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
            case PinType::Bool:
                return storage.bool_ptr && storage.bool_ptr[0] ? "true" : "false";
            case PinType::Int:
                return storage.ptr ? Strings::format("{}", storage.int_ptr[0]) : "0";
            case PinType::UInt:
                return storage.ptr ? Strings::format("{}", storage.uint_ptr[0]) : "0";
            case PinType::Float:
                return storage.ptr ? Strings::format("{:.6f}f", storage.float_ptr[0]) : "0.f";
            case PinType::BVec2:
                break;
            case PinType::BVec3:
                break;
            case PinType::BVec4:
                break;
            case PinType::IVec2:
                break;
            case PinType::IVec3:
                break;
            case PinType::IVec4:
                break;
            case PinType::UVec2:
                break;
            case PinType::UVec3:
                break;
            case PinType::UVec4:
                break;

            case PinType::Vec2:
                return storage.float_ptr ? Strings::format("float2({:.6f}, {:.6f})", storage.float_ptr[0], storage.float_ptr[1])
                                         : "float2(0.f, 0.f)";
            case PinType::Vec3:
            case PinType::Color3:
                return storage.float_ptr ? Strings::format("float3({:.6f}, {:.6f}, {:.6f})", storage.float_ptr[0],
                                                           storage.float_ptr[1], storage.float_ptr[2])
                                         : "float3(0.f, 0.f, 0.f)";
            case PinType::Vec4:
            case PinType::Color4:
                return storage.float_ptr ? Strings::format("float4({:.6f}, {:.6f}, {:.6f}, {:.6f})", storage.float_ptr[0],
                                                           storage.float_ptr[1], storage.float_ptr[2], storage.float_ptr[3])
                                         : "float4(0.f, 0.f, 0.f, 0.f)";

            default:
                return "";
        }

        return "";
    }


    //////////////// NODES IMPLEMENTATION ////////////////

    implement_visual_material_node(Root, Init);

    Root::Root()
    {
        m_inputs.push_back(new Color3InputPin(this, "Base Color"));
        m_inputs.push_back(new FloatInputPin(this, "Opacity"));
        m_inputs.push_back(new Color3InputPin(this, "Emissive"));
        m_inputs.push_back(new FloatInputPin(this, "Specular"));
        m_inputs.push_back(new FloatInputPin(this, "Metalness"));
        m_inputs.push_back(new FloatInputPin(this, "Roughness"));
        m_inputs.push_back(new FloatInputPin(this, "AO"));
        m_inputs.push_back(new Vec3InputPinND(this, "Normal"));
        m_inputs.push_back(new Vec3InputPin(this, "Position Offset"));
    }

    Expression Root::compile(InputPin* pin, CompilerState& state)
    {
        auto expression = Node::compile(pin, state);
        return state.expression_cast(expression, pin->type());
    }

    bool Root::is_destroyable() const
    {
        return false;
    }


#define implement_visual_material_constant_node(type)                                                                            \
    implement_visual_material_node(type, Constants);                                                                             \
    type::type()                                                                                                                 \
    {                                                                                                                            \
        m_outputs.push_back(new type##OutputPin(this, "Value"));                                                                 \
    }                                                                                                                            \
    Expression type::compile(OutputPin* pin, CompilerState& state)                                                               \
    {                                                                                                                            \
        return Expression(create_default_value(PinType::type, pin->default_value()), PinType::type);                             \
    }

    implement_visual_material_constant_node(Bool);
    implement_visual_material_constant_node(Int);
    implement_visual_material_constant_node(UInt);
    implement_visual_material_constant_node(Float);
    implement_visual_material_constant_node(BVec2);
    implement_visual_material_constant_node(BVec3);
    implement_visual_material_constant_node(BVec4);
    implement_visual_material_constant_node(IVec2);
    implement_visual_material_constant_node(IVec3);
    implement_visual_material_constant_node(IVec4);
    implement_visual_material_constant_node(UVec2);
    implement_visual_material_constant_node(UVec3);
    implement_visual_material_constant_node(UVec4);
    implement_visual_material_constant_node(Vec2);
    implement_visual_material_constant_node(Vec3);
    implement_visual_material_constant_node(Vec4);
    implement_visual_material_constant_node(Color3);
    implement_visual_material_constant_node(Color4);

}// namespace Engine::VisualMaterialGraph
