#pragma once
#include <Graphics/shader_compiler.hpp>
#include <slang-com-ptr.h>
#include <slang.h>
#include <spirv_glsl.hpp>


namespace Engine
{
	class Material;
	class Pipeline;
	class Shader;

	class SLANG_ShaderCompiler : public ShaderCompiler
	{
		trinex_declare_class(SLANG_ShaderCompiler, ShaderCompiler);

	public:
		struct ShaderInfo {
			Buffer* source;
			const char* entry_name    = nullptr;
			slang::IEntryPoint* entry = nullptr;
			int32_t index             = -1;
		};

		class Context
		{
		public:
			using CheckStages = bool (*)(ShaderInfo*);

			slang::IModule* module = nullptr;
			Vector<slang::IComponentType*> component_types;
			SLANG_ShaderCompiler* const compiler;
			Context* const prev_ctx;

			bool compile(ShaderInfo* infos, size_t len);
		};

		struct SessionInitializer {
			Vector<const char*, FrameAllocator<const char*>> search_paths;
			Vector<slang::PreprocessorMacroDesc, FrameAllocator<slang::PreprocessorMacroDesc>> definitions;
			Vector<slang::CompilerOptionEntry, FrameAllocator<slang::CompilerOptionEntry>> options;
			Vector<slang::CompilerOptionEntry, FrameAllocator<slang::CompilerOptionEntry>> target_options;

			slang::SessionDesc session_desc;
			slang::TargetDesc target_desc;

			FORCE_INLINE void add_definition(const char* key, const char* value) { definitions.push_back({key, value}); }
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
		static inline SLANG_ShaderCompiler* instance(const StringView& rhi = "")
		{
			return instance_cast<SLANG_ShaderCompiler>(Super::instance(rhi));
		}

		SLANG_ShaderCompiler();
		~SLANG_ShaderCompiler();
		SLANG_ShaderCompiler& on_create() override;
		virtual void initialize_context(SessionInitializer* session);
		virtual bool submit_result(ShaderCompilationResult& result);
		bool compile(const ShaderCompilationEnvironment* env, ShaderCompilationResult& result) override;

		inline slang::IModule* load_module(const char* module) { return m_session->loadModule(module); }
	};

	class NONE_ShaderCompiler : public SLANG_ShaderCompiler
	{
		trinex_declare_class(NONE_ShaderCompiler, SLANG_ShaderCompiler);

	public:
		void initialize_context(SessionInitializer* session) override;
	};

	class VULKAN_ShaderCompiler : public SLANG_ShaderCompiler
	{
		trinex_declare_class(VULKAN_ShaderCompiler, SLANG_ShaderCompiler);

	private:
		struct SPIRV {
			enum Opcodes
			{
				OpDecorate = 71,
				OpVariable = 59,
			};

			static constexpr uint32_t Decoration_Location = 30;
			static constexpr uint32_t StorageClass_Input  = 1;

			static inline uint16_t wordcount(uint32_t w) { return static_cast<uint16_t>(w >> 16); }
			static inline uint16_t opcode(uint32_t w) { return static_cast<uint16_t>(w & 0xFFFF); }
		};

		static bool strip_vertex_inputs(const uint32_t* spirv, const uint32_t words, Vector<RHIVertexAttribute>& attributes);

	public:
		void initialize_context(SessionInitializer* session) override;
		bool submit_result(ShaderCompilationResult& result) override;
	};

	class D3D12_ShaderCompiler : public SLANG_ShaderCompiler
	{
		trinex_declare_class(D3D12_ShaderCompiler, SLANG_ShaderCompiler);

	public:
		void initialize_context(SessionInitializer* session) override;
	};
}// namespace Engine
