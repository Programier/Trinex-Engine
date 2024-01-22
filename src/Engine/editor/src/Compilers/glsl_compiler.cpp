#include <Compiler/glsl_compiler.hpp>
#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Graphics/material_nodes.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/visual_material.hpp>

namespace Engine
{

    static Map<MaterialNodes::Type, NodeInfo>& node_info()
    {
        static Map<MaterialNodes::Type, NodeInfo> functions;
        return functions;
    }

    static CompiledNode& create_node(CompiledNodes& compiled, Node* node)
    {
        auto& res = compiled[node->id];
        if (res.pin_info.size() < node->output.size())
            res.pin_info.resize(node->output.size());
        return res;
    }


    static NodePin::DataType deduce_pin_type(OutputPin* pin, CompiledNodes& compiled)
    {
        if (pin == nullptr)
            return NodePin::Undefined;

        auto& result = create_node(compiled, pin->node).pin_info[pin->index].type;
        if (result != NodePin::Undefined)
            return result;

        auto func = node_info()[pin->node->type()].deduce;
        if (func)
        {
            result = static_cast<NodePin::DataType>(func(pin, compiled));
        }
        else
        {
            result = static_cast<NodePin::DataType>(pin->data_types);
        }

        return result;
    }

    static const char* glsl_type_of(EnumerateType _type)
    {
        NodePin::DataType type = static_cast<NodePin::DataType>(_type);
        switch (type)
        {
            case NodePin::DataType::Bool:
                return "bool";
            case NodePin::DataType::Int:
                return "int";
            case NodePin::DataType::UInt:
                return "uint";
            case NodePin::DataType::Float:
                return "float";
            case NodePin::DataType::BVec2:
                return "bvec2";
            case NodePin::DataType::BVec3:
                return "bvec3";
            case NodePin::DataType::BVec4:
                return "bvec4";
            case NodePin::DataType::IVec2:
                return "ivec2";
            case NodePin::DataType::IVec3:
                return "ivec3";
            case NodePin::DataType::IVec4:
                return "ivec4";
            case NodePin::DataType::UVec2:
                return "uvec2";
            case NodePin::DataType::UVec3:
                return "uvec3";
            case NodePin::DataType::UVec4:
                return "uvec4";
            case NodePin::DataType::Vec2:
                return "vec2";
            case NodePin::DataType::Vec3:
                return "vec3";
            case NodePin::DataType::Vec4:
                return "vec4";
            case NodePin::DataType::Color3:
                return "vec3";
            case NodePin::DataType::Color4:
                return "vec4";
            default:
                return nullptr;
        }
    }

    PinInfo::operator const String&() const
    {
        return code;
    }

    PinInfo::operator String&()
    {
        return code;
    }

    NodeInfo& NodeInfo::operator=(void (*callback)(NodeInfo* info))
    {
        callback(this);
        return *this;
    }

    void ShaderCode::clear()
    {
        inputs.clear();
        outputs.clear();
        main.clear();
        next_var_id = 1;
        compiled_nodes.clear();
    }

    String ShaderCode::output() const
    {
        String out = "#version 310 es\nprecision highp float;\n";

        for (const String& input : inputs)
        {
            out += input;
            out += ";\n";
        }

        out += "\n\nvoid main()\n{\n";

        for (const String& main_sentence : main)
        {
            out.push_back('\t');
            out += main_sentence;
            out += ";\n";
        }

        out += "}\n";

        return out;
    }

    String ShaderCode::create_variable(OutputPin* pin, const String& compiled)
    {
        String name      = Strings::format("var_{}", next_var_id++);
        const char* type = glsl_type_of(deduce_pin_type(pin, compiled_nodes));
        if (type == nullptr)
        {
            compiler->is_build_fail = true;
            compiler->errors->push_back("Failed to determine node output type");
            error_log("GLSL Compiler", "Failed to get type of output pin");
        }
        else
        {
            main.push_back(Strings::format("{} {} = {}", type, name, compiled));
        }
        return name;
    }

    ShaderCode& ShaderCode::submit(OutputPin* pin, const String& compiled, String& out)
    {
        if (pin->has_one_output_link())
        {
            out = compiled;
        }
        else
        {
            out = create_variable(pin, compiled);
        }
        return *this;
    }

    ShaderCode& ShaderCode::push_input(const char* type, const char* input)
    {
        Index location = inputs.size();
        inputs.emplace_back(Strings::format("layout(location = {}) in {} in_{}", location, type, input));
        return *this;
    }

    ShaderCode& ShaderCode::push_output(const char* type, const char* output)
    {
        Index location = inputs.size();
        outputs.emplace_back(Strings::format("layout(location = {}) out {} in_{}", location, type, output));
        return *this;
    }


    GLSL_Compiler::GLSL_Compiler()
    {
        vertex.compiler   = this;
        fragment.compiler = this;
    }

    String GLSL_Compiler::default_value_of(void* data, NodePin::DataType type)
    {
        if (data == nullptr)
        {
            errors->push_back("Pin of node is empty!");
            is_build_fail = true;
            return "";
        }

        switch (type)
        {
            case NodePin::DataType::Bool:
                return (*reinterpret_cast<bool*>(data)) ? "true" : "false";
            case NodePin::DataType::Int:
                return Strings::format("{}", *reinterpret_cast<int_t*>(data));
            case NodePin::DataType::UInt:
                return Strings::format("{}", *reinterpret_cast<uint_t*>(data));
            case NodePin::DataType::Float:
                return Strings::format("{}", *reinterpret_cast<float*>(data));

            case NodePin::DataType::BVec2:
                break;
            case NodePin::DataType::BVec3:
                break;
            case NodePin::DataType::BVec4:
                break;

            case NodePin::DataType::IVec2:
            {
                IntVector2D* vec = reinterpret_cast<IntVector2D*>(data);
                return Strings::format("ivec2({}, {})", vec->x, vec->y);
            }

            case NodePin::DataType::IVec3:
            {
                IntVector3D* vec = reinterpret_cast<IntVector3D*>(data);
                return Strings::format("ivec3({}, {}, {})", vec->x, vec->y, vec->z);
            }

            case NodePin::DataType::IVec4:
            {
                IntVector4D* vec = reinterpret_cast<IntVector4D*>(data);
                return Strings::format("ivec4({}, {}, {}, {})", vec->x, vec->y, vec->z, vec->w);
            }

            case NodePin::DataType::UVec2:
            {
                UIntVector2D* vec = reinterpret_cast<UIntVector2D*>(data);
                return Strings::format("uvec2({}, {})", vec->x, vec->y);
            }

            case NodePin::DataType::UVec3:
            {
                UIntVector3D* vec = reinterpret_cast<UIntVector3D*>(data);
                return Strings::format("uvec3({}, {}, {})", vec->x, vec->y, vec->z);
            }

            case NodePin::DataType::UVec4:
            {
                UIntVector4D* vec = reinterpret_cast<UIntVector4D*>(data);
                return Strings::format("uvec4({}, {}, {}, {})", vec->x, vec->y, vec->z, vec->w);
            }

            case NodePin::DataType::Vec2:
            {
                Vector2D* vec = reinterpret_cast<Vector2D*>(data);
                return Strings::format("vec2({}, {})", vec->x, vec->y);
            }

            case NodePin::DataType::Vec3:
            {
                Vector3D* vec = reinterpret_cast<Vector3D*>(data);
                return Strings::format("vec3({}, {}, {})", vec->x, vec->y, vec->z);
            }

            case NodePin::DataType::Vec4:
            {
                Vector4D* vec = reinterpret_cast<Vector4D*>(data);
                return Strings::format("vec4({}, {}, {}, {})", vec->x, vec->y, vec->z, vec->w);
            }

            case NodePin::DataType::Color3:
            {
                Vector3D* vec = reinterpret_cast<Vector3D*>(data);
                return Strings::format("vec3({}, {}, {})", vec->x, vec->y, vec->z);
            }

            case NodePin::DataType::Color4:
            {
                Vector4D* vec = reinterpret_cast<Vector4D*>(data);
                return Strings::format("vec4({}, {}, {}, {})", vec->x, vec->y, vec->z, vec->w);
            }
            default:
                throw EngineException("Undefined type");
        }

        return "";
    }

    const CompiledNode& GLSL_Compiler::compile_node(Node* node, ShaderCode* code)
    {
        {
            auto it = code->compiled_nodes.find(node->id);
            if (it != code->compiled_nodes.end() && it->second.is_compiled)
                return it->second;
        }

        NodeCompilerFunction func = node_info()[node->type()].compile;

        CompiledNode& out = create_node(code->compiled_nodes, node);

        {
            for (OutputPin* pin : node->output)
            {
                deduce_pin_type(pin, code->compiled_nodes);
            }
        }

        if (func)
        {
            func(this, node, out, code);
        }
        else
        {
            for (OutputPin* pin : node->output)
            {
                void* default_value = pin->default_value();
                if (default_value)
                {
                    out.pin_info[pin->index].code =
                            default_value_of(default_value, static_cast<NodePin::DataType>(pin->data_types));
                }
                else if (!is_build_fail)
                {
                    is_build_fail = true;
                    error_log("GLSL Compiler", "Failed to find function for node '%d'", node->type());
                }
            }
        }

        out.is_compiled = true;

        return out;
    }

    String GLSL_Compiler::pin_source(InputPin* pin, ShaderCode* code, NodePin::DataType out_type)
    {
        if (pin->linked_to)
        {
            auto& result = compile_node(pin->linked_to->node, code).pin_info[pin->linked_to->index];
            if (out_type != NodePin::Undefined && (result.type & out_type) != out_type)
            {
                return Strings::format("{}({})", glsl_type_of(out_type), result.code);
            }
            return result.code;
        }

        String result = default_value_of(pin->default_value(), static_cast<NodePin::DataType>(pin->data_types));
        if (out_type != NodePin::Undefined && (pin->data_types & out_type) != out_type)
        {
            return Strings::format("{}({})", glsl_type_of(pin->data_types), result);
        }
        return result;
    }


    GLSL_Compiler& GLSL_Compiler::compile_vertex_shader(VisualMaterial* material)
    {
        Node* root = material->vertex_node();
        vertex.main.push_back(Strings::format("glPosition = vec4({}, 1.0)", pin_source(root->input[0], &vertex, NodePin::Vec3)));
        return *this;
    }

    GLSL_Compiler& GLSL_Compiler::compile_fragment_shader(VisualMaterial* material)
    {
        fragment.push_output("vec3", "color");

        Node* root = material->fragment_node();
        fragment.main.push_back(Strings::format("out_color = {}", pin_source(root->input[0], &fragment, NodePin::Vec3)));
        return *this;
    }

    bool GLSL_Compiler::compile(VisualMaterial* material, MessageList& errors)
    {
        fragment.clear();
        vertex.clear();
        this->errors  = &errors;
        is_build_fail = false;

        compile_vertex_shader(material);
        compile_fragment_shader(material);

        material->pipeline->vertex_shader->text_code   = vertex.output();
        material->pipeline->fragment_shader->text_code = fragment.output();

        if (is_build_fail)
        {
            error_log("GLSL Compiler", "Failed to build shader!");
        }
        return !is_build_fail;
    }

    implement_engine_class_default_init(GLSL_Compiler);


    static NodePin::DataType default_deduce_func(OutputPin* pin, CompiledNodes& nodes)
    {
        InputPin* in_pin = pin->node->input[0];
        if (in_pin->linked_to)
            return deduce_pin_type(in_pin->linked_to, nodes);

        return static_cast<NodePin::DataType>(in_pin->data_types);
    }

#define compile_func() info->compile = [](GLSL_Compiler * compiler, Node * node, CompiledNode & out, ShaderCode * code)
#define deduce_func() info->deduce = [](OutputPin * pin, CompiledNodes & nodes) -> EnumerateType
#define deduce_func_default() info->deduce = default_deduce_func
#define implement_node(node_name) node_info()[MaterialNodes::Type::node_name] = [](NodeInfo * info)


    static PostInitializeController initializer([]() {
        implement_node(Sin)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("sin({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Cos)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("cos({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Tan)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("tan({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(ASin)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("asin({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(ACos)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("acos({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(ATan)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("atan({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(SinH)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("sinh({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(CosH)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("cosh({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(TanH)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("tanh({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(ASinH)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("asinh({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(ACosH)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("acosh({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(ATanH)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("atanh({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Pow)
        {
            compile_func()
            {
                code->submit(node->output[0],
                             Strings::format("pow({}, {})", compiler->pin_source(node->input[0], code),
                                             compiler->pin_source(node->input[1], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Exp)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("exp({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Log)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("log({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Exp2)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("exp2({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Log2)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("log2({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };


        implement_node(Sqrt)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("sqrt({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(InverseSqrt)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("inversesqrt({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Abs)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("abs({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Sign)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("sign({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Floor)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("floor({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Ceil)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("ceil({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Fract)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("fract({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Mod)
        {
            compile_func()
            {
                code->submit(node->output[0], Strings::format("mod({})", compiler->pin_source(node->input[0], code)),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };


        implement_node(Max)
        {
            compile_func()
            {
                code->submit(node->output[0],
                             Strings::format("max({}, {})", compiler->pin_source(node->input[0], code),
                                             compiler->pin_source(node->input[1], code,
                                                                  deduce_pin_type(node->output[0], code->compiled_nodes))),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        implement_node(Min)
        {
            compile_func()
            {
                code->submit(node->output[0],
                             Strings::format("min({}, {})", compiler->pin_source(node->input[0], code),
                                             compiler->pin_source(node->input[1], code,
                                                                  deduce_pin_type(node->output[0], code->compiled_nodes))),
                             out.pin_info[0]);
            };

            deduce_func_default();
        };

        //                Clamp,
        //                Mix,
        //                Step,
        //                Smoothstep
    });
}// namespace Engine
