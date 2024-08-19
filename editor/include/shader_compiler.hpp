#pragma once
#include <Graphics/shader_compiler.hpp>

namespace Engine
{
	class Material;

	namespace ShaderCompiler
	{
		class OPENGL_Compiler : public Compiler
		{
			declare_class(OPENGL_Compiler, Compiler);

		public:
			bool compile(Material* material, const String& slang_source, ShaderSource& out_source, MessageList& errors) override;
		};

		class VULKAN_Compiler : public Compiler
		{
			declare_class(VULKAN_Compiler, Compiler);

		public:
			bool compile(Material* material, const String& slang_source, ShaderSource& out_source, MessageList& errors) override;
		};

		class NONE_Compiler : public Compiler
		{
			declare_class(NONE_Compiler, Compiler);

		public:
			bool compile(Material* material, const String& slang_source, ShaderSource& out_source, MessageList& errors) override;
		};

		class D3D11_Compiler : public Compiler
		{
			declare_class(D3D11_Compiler, Compiler);

		public:
			bool compile(Material* material, const String& slang_source, ShaderSource& out_source, MessageList& errors) override;
		};
	}// namespace ShaderCompiler
}// namespace Engine
