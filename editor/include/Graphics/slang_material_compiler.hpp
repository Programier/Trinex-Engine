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

	using SLANG_DefinitionsArray = Containers::Vector<ShaderDefinition, FrameAllocator<ShaderDefinition>>;

	class SLANG_MaterialCompiler : public MaterialCompiler
	{
		trinex_declare_class(SLANG_MaterialCompiler, MaterialCompiler);

	public:
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

			SLANG_DefinitionsArray definitions;
			Slang::ComPtr<slang::IModule> module;
			SLANG_MaterialCompiler* const compiler;
			Context* const prev_ctx;

			size_t calculate_source_len(const String& source);
			char* initialize_definitions(char* dst);
			bool initialize(const String& source, Pipeline* pipeline);
			bool compile(ShaderInfo* infos, size_t len, Pipeline* pipeline, CheckStages checker);

		public:
			Context(SLANG_MaterialCompiler* compiler);
			bool compile_graphics(const String& source, Material* material, Refl::RenderPassInfo* pass);
			bool compile_graphics(const String& source, Pipeline* pipeline, Refl::RenderPassInfo* pass = nullptr);
			bool compile_compute(const String& source, Pipeline* pipeline);
			~Context();
		};

		struct SessionInitializer {
			Containers::Vector<const char*, FrameAllocator<const char*>> search_paths;
			Containers::Vector<slang::PreprocessorMacroDesc, FrameAllocator<slang::PreprocessorMacroDesc>> definitions;
			Containers::Vector<slang::CompilerOptionEntry, FrameAllocator<slang::CompilerOptionEntry>> options;
			Containers::Vector<slang::CompilerOptionEntry, FrameAllocator<slang::CompilerOptionEntry>> target_options;

			slang::SessionDesc session_desc;
			slang::TargetDesc target_desc;

			FORCE_INLINE void add_definition(const char* key, const char* value) { definitions.push_back({key, value}); }
			FORCE_INLINE void add_definition(const ShaderDefinition& definition)
			{
				add_definition(definition.key.c_str(), definition.value.c_str());
			}

			FORCE_INLINE void add_search_path(const char* path) { search_paths.push_back(path); }
			FORCE_INLINE void add_option(slang::CompilerOptionName name, bool v1, bool v2 = false)
			{
				add_option(name, v1 ? 1 : 0, v2 ? 1 : 0);
			}

			FORCE_INLINE void add_option(slang::CompilerOptionName name, uint32_t v1, uint32_t v2 = 0)
			{
				add_option(name, static_cast<int32_t>(v1), static_cast<int32_t>(v2));
			}

			FORCE_INLINE void add_option(slang::CompilerOptionName name, int32_t v1, int32_t v2 = 0)
			{
				slang::CompilerOptionEntry entry;
				entry.name            = name;
				entry.value.kind      = slang::CompilerOptionValueKind::Int;
				entry.value.intValue0 = v1;
				entry.value.intValue1 = v2;
				options.push_back(entry);
			}

			FORCE_INLINE void add_option(slang::CompilerOptionName name, const char* v1, const char* v2 = nullptr)
			{
				slang::CompilerOptionEntry entry;
				entry.name               = name;
				entry.value.kind         = slang::CompilerOptionValueKind::String;
				entry.value.stringValue0 = v1;
				entry.value.stringValue1 = v2;
				options.push_back(entry);
			}

			FORCE_INLINE void add_target_option(slang::CompilerOptionName name, bool v1, bool v2 = false)
			{
				add_target_option(name, v1 ? 1 : 0, v2 ? 1 : 0);
			}

			FORCE_INLINE void add_target_option(slang::CompilerOptionName name, uint32_t v1, uint32_t v2 = 0)
			{
				add_target_option(name, static_cast<int32_t>(v1), static_cast<int32_t>(v2));
			}

			FORCE_INLINE void add_target_option(slang::CompilerOptionName name, int32_t v1, int32_t v2 = 0)
			{
				slang::CompilerOptionEntry entry;
				entry.name            = name;
				entry.value.kind      = slang::CompilerOptionValueKind::Int;
				entry.value.intValue0 = v1;
				entry.value.intValue1 = v2;
				target_options.push_back(entry);
			}

			FORCE_INLINE void add_target_option(slang::CompilerOptionName name, const char* v1, const char* v2 = nullptr)
			{
				slang::CompilerOptionEntry entry;
				entry.name               = name;
				entry.value.kind         = slang::CompilerOptionValueKind::String;
				entry.value.stringValue0 = v1;
				entry.value.stringValue1 = v2;
				target_options.push_back(entry);
			}
		};

	protected:
		Slang::ComPtr<slang::ISession> m_session;
		Context* m_ctx = nullptr;

	public:
		SLANG_MaterialCompiler();
		SLANG_MaterialCompiler& on_create() override;
		virtual void initialize_context(SessionInitializer* session);
		virtual void submit_source(Shader* shader, const byte* src, size_t size);
		bool compile(Material* material) override;
		bool compile_pass(Material* material, Refl::RenderPassInfo* pass) override;
		bool compile_pass(Material* material, Refl::RenderPassInfo* pass, const String& source);
		bool compile(const String& source, Pipeline* pipeline) override;
	};

	class NONE_MaterialCompiler : public Singletone<NONE_MaterialCompiler, SLANG_MaterialCompiler>
	{
		trinex_declare_class(NONE_MaterialCompiler, SLANG_MaterialCompiler);

	public:
		void initialize_context(SessionInitializer* session) override;
	};

	class VULKAN_MaterialCompiler : public Singletone<VULKAN_MaterialCompiler, SLANG_MaterialCompiler>
	{
		trinex_declare_class(VULKAN_MaterialCompiler, SLANG_MaterialCompiler);

	public:
		void initialize_context(SessionInitializer* session) override;
	};

	class OPENGL_MaterialCompiler : public Singletone<OPENGL_MaterialCompiler, SLANG_MaterialCompiler>
	{
		trinex_declare_class(OPENGL_MaterialCompiler, SLANG_MaterialCompiler);

	public:
		void submit_source(Shader* shader, const byte* src, size_t size) override;
		void initialize_context(SessionInitializer* session) override;
	};

	class D3D11_MaterialCompiler : public Singletone<D3D11_MaterialCompiler, SLANG_MaterialCompiler>
	{
		trinex_declare_class(D3D11_MaterialCompiler, SLANG_MaterialCompiler);

	public:
		void initialize_context(SessionInitializer* session) override;
	};
}// namespace Engine
