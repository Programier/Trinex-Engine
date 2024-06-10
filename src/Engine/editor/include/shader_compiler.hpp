#pragma once
#include <Graphics/shader_compiler.hpp>

namespace Engine
{
    class Material;

    namespace ShaderCompiler
    {
        class OpenGL_Compiler : public Compiler
        {
            declare_class(OpenGL_Compiler, Compiler);

        public:
            bool compile(Material* material, const String& slang_source, ShaderSource& out_source, MessageList& errors) override;
        };

        class Vulkan_Compiler : public Compiler
        {
            declare_class(Vulkan_Compiler, Compiler);

        public:
            bool compile(Material* material, const String& slang_source, ShaderSource& out_source, MessageList& errors) override;
        };
    }// namespace ShaderCompiler
}// namespace Engine
