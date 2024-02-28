#pragma once
#include <Core/engine_types.hpp>
#include <Graphics/visual_material.hpp>


namespace Engine
{
    class Texture2D;
    class Sampler;
}// namespace Engine

namespace Engine::MaterialNodes
{
#define declare_material_node()                                                                                                  \
    static Struct* static_struct_instance;                                                                                       \
    Struct* struct_instance() const override;                                                                                    \
    const char* name() const override;

    struct VertexNode : public MaterialNode {
        declare_material_node();

        VertexNode();
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
        bool is_removable() const override;
        void bind_to_properties_window(ImGuiObjectProperties*) override;
    };

    struct FragmentNode : public MaterialNode {
        declare_material_node();

        FragmentNode();
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
        bool is_removable() const override;
        void bind_to_properties_window(ImGuiObjectProperties*) override;
    };


    struct Sampler : public MaterialNode {
        declare_material_node();
        Pointer<Engine::Sampler> sampler;

        Sampler();
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
        bool archive_process(Archive& ar) override;
    };

    struct Texture2D : public MaterialNode {
        declare_material_node();
        Pointer<Engine::Texture2D> texture;

        Texture2D();
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
        void render() override;
        bool archive_process(Archive& ar) override;
    };

    struct MaterialDynamicParameter : public MaterialNode {
        String node_name;

        void render() override;
        bool archive_process(Archive& ar) override;
    };

    struct CustomCode : public MaterialNode {
        declare_material_node();
        Vector<MaterialNodeDataType> output_types;
        Path shader_path;

        void bind_to_properties_window(ImGuiObjectProperties* props) override;
        MaterialNodeDataType output_type(const MaterialOutputPin* pin) const override;
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
        bool serialize_pins() const override;
        bool archive_process(Archive& ar) override;
    };
}// namespace Engine::MaterialNodes
