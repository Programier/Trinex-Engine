#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/structures.hpp>
#include <Graphics/material_compiler.hpp>
#include <slang-com-ptr.h>
#include <slang.h>
#include <spirv_glsl.hpp>


namespace Engine
{
	class Material;
	class Pipeline;
	class Shader;

	class SLANG_MaterialCompiler : public MaterialCompiler
	{
		declare_class(SLANG_MaterialCompiler, MaterialCompiler);

	protected:
		class Context
		{
		public:
			struct ShaderInfo {
				ShaderType type;
				const char* entry_name;
				Slang::ComPtr<slang::IEntryPoint> entry;
				int32_t index = -1;
			};

			using CheckStages = bool (*)(ShaderInfo*);

			Slang::ComPtr<slang::ISession> session;
			Slang::ComPtr<SlangCompileRequest> compile_request;
			SLANG_MaterialCompiler* const compiler;
			Context* const prev_ctx;
			int32_t unit = 0;

		public:
			Context(SLANG_MaterialCompiler* compiler);
			bool initialize(const String& source);
			bool compile(ShaderInfo* infos, size_t len, Pipeline* pipeline, CheckStages checker);
			bool compile_graphics(Material* material, Refl::RenderPassInfo* pass);
			bool compile_graphics(Pipeline* pipeline);
			bool compile_compute(Pipeline* pipeline);

			FORCE_INLINE void add_definition(const char* key, const char* value)
			{
				compile_request->addPreprocessorDefine(key, value);
			}

			FORCE_INLINE void add_definition(const ShaderDefinition& definition)
			{
				add_definition(definition.key.c_str(), definition.value.c_str());
			}

			~Context();
		};

		Vector<Path> m_include_directories;
		Context* m_ctx = nullptr;

	public:
		SLANG_MaterialCompiler();
		virtual void initialize_context();
		virtual void submit_source(Shader* shader, const byte* src, size_t size);
		bool compile(Material* material) override;
		bool compile_pass(Material* material, Refl::RenderPassInfo* pass) override;
		bool compile_pass(Material* material, Refl::RenderPassInfo* pass, const String& source);
		bool compile(const String& source, Pipeline* pipeline) override;
	};

	class NONE_MaterialCompiler : public Singletone<NONE_MaterialCompiler, SLANG_MaterialCompiler>
	{
		declare_class(NONE_MaterialCompiler, SLANG_MaterialCompiler);

	public:
		void initialize_context() override;
	};

	class VULKAN_MaterialCompiler : public Singletone<VULKAN_MaterialCompiler, SLANG_MaterialCompiler>
	{
		declare_class(VULKAN_MaterialCompiler, SLANG_MaterialCompiler);

	public:
		void initialize_context() override;
	};

	class OPENGL_MaterialCompiler : public Singletone<OPENGL_MaterialCompiler, SLANG_MaterialCompiler>
	{
		declare_class(OPENGL_MaterialCompiler, SLANG_MaterialCompiler);

	public:
		void submit_source(Shader* shader, const byte* src, size_t size) override;
		void initialize_context() override;
	};

	class D3D11_MaterialCompiler : public Singletone<D3D11_MaterialCompiler, SLANG_MaterialCompiler>
	{
		declare_class(D3D11_MaterialCompiler, SLANG_MaterialCompiler);

	public:
		void initialize_context() override;
	};
}// namespace Engine
