#pragma once
#include <Core/engine_types.hpp>
#include <Graphics/visual_material.hpp>

namespace Engine::MaterialNodes
{
    struct VertexNode : public MaterialNode {
        static Struct* static_struct_instance;

        Struct* struct_instance() const override;
        const char* name() const override;
        VertexNode();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
    };

    struct FragmentNode : public MaterialNode {
        static Struct* static_struct_instance;

        Struct* struct_instance() const override;
        const char* name() const override;
        FragmentNode();

        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
    };
}// namespace Engine::MaterialNodes
