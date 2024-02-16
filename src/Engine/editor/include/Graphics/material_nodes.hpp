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
    };

    struct FragmentNode : public MaterialNode {
        declare_material_node();

        FragmentNode();
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
        bool is_removable() const override;
    };


    struct Sampler : public MaterialNode {
        declare_material_node();
        Engine::Sampler* sampler = nullptr;

        Sampler();
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
    };

    struct Texture2D : public MaterialNode {
        declare_material_node();
        Engine::Texture2D* texture = nullptr;

        Texture2D();
        size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin) override;
        void render() override;
    };
}// namespace Engine::MaterialNodes
