#pragma once
#include <Core/object.hpp>

namespace Engine
{
	class Logger;
	class Material;
	class Pipeline;
	class Path;
	class RenderPass;

	class ShaderCompilationEnvironment
	{
	public:
		virtual ShaderCompilationEnvironment& add_module(const char* module) = 0;
	};

	class ENGINE_EXPORT ShaderCompiler : public Object
	{
		trinex_declare_class(ShaderCompiler, Object);

	public:
		static ShaderCompiler* instance(const StringView& api_name = "");
		virtual bool compile(Material* material)                               = 0;
		virtual bool compile(const String& source, Pipeline* pipeline)         = 0;
		virtual bool compile_pass(Material* material, RenderPass* render_pass) = 0;
	};

}// namespace Engine
