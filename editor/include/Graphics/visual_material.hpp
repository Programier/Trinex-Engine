#pragma once
#include <Core/flags.hpp>
#include <Graphics/material.hpp>

namespace Engine
{
    namespace VisualMaterialGraph
    {
        class Node;
    }

    class ENGINE_EXPORT VisualMaterial : public Material
    {
        declare_class(VisualMaterial, Material);

    private:
        Vector<Pointer<VisualMaterialGraph::Node>> m_nodes;

    public:
        MaterialDomain domain;

        VisualMaterial();
        const Vector<Pointer<VisualMaterialGraph::Node>>& nodes() const;
        VisualMaterialGraph::Node* create_node(class Class* node_class);
        VisualMaterial& destroy_node(VisualMaterialGraph::Node* node);
        bool shader_source(String& out_source) override;
        bool compile(ShaderCompiler::Compiler* compiler = nullptr, MessageList* errors = nullptr) override;
    };
}// namespace Engine
