#pragma once
#include <Core/object.hpp>
#include <Core/structures.hpp>

namespace Engine
{
	class Logger;
	class Material;
	class Pipeline;

	class ENGINE_EXPORT MaterialCompiler : public Object
	{
		declare_class(MaterialCompiler, Object);

	public:
		static MaterialCompiler* instance(const StringView& api_name = "");
		virtual bool compile(Material* material)                                         = 0;
		virtual bool compile(const String& source, Pipeline* pipeline)                   = 0;
		virtual bool compile_pass(Material* material, Refl::RenderPassInfo* render_pass) = 0;
	};

}// namespace Engine
