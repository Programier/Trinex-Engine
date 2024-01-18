#pragma once
#include <Compiler/compiler.hpp>
#include <Graphics/visual_material.hpp>


namespace Engine
{

    using NodeCompilerFunction = void (*)(class GLSL_Compiler* compiler, struct Node* node, struct CompiledNode& out,
                                          struct ShaderCode* code);

    using CompiledNodes         = Map<Identifier, CompiledNode>;
    using DeducePinTypeFunction = EnumerateType (*)(OutputPin* pin, CompiledNodes& nodes);

    struct NodeInfo {
        NodeCompilerFunction compile = nullptr;
        DeducePinTypeFunction deduce = nullptr;

        NodeInfo& operator=(void (*callback)(NodeInfo* info));
    };

    struct PinInfo {
        String code;
        NodePin::DataType type = NodePin::Undefined;

        operator const String&() const;
        operator String&();
    };

    struct CompiledNode {
        Vector<PinInfo> pin_info;
        bool is_compiled = false;
    };


    struct ShaderCode {
        CompiledNodes compiled_nodes;
        class GLSL_Compiler* compiler;

        Vector<String> globals;
        Vector<String> main;

        Identifier next_var_id = 1;

        void clear();
        String output() const;
        String create_variable(struct OutputPin* pin, const String& compiled);
        ShaderCode& submit(struct OutputPin* pin, const String& compiled, String& out);
    };

    class GLSL_Compiler : public MaterialCompiler
    {
        declare_class(GLSL_Compiler, MaterialCompiler);

    public:
        ShaderCode vertex;
        ShaderCode fragment;
        MessageList* errors;
        bool is_build_fail = false;

        GLSL_Compiler();
        String default_value_of(void* data, NodePin::DataType type);
        const CompiledNode& compile_node(Node* node, ShaderCode* code);
        String pin_source(InputPin* pin, ShaderCode* code, NodePin::DataType out_type = NodePin::DataType::Undefined);
        bool compile(VisualMaterial* material, MessageList& errors) override;
    };
}// namespace Engine
