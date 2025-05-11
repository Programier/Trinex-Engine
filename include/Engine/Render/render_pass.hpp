#pragma once
#include <Core/name.hpp>

namespace Engine
{
	class Material;
	class ShaderCompilationEnvironment;

	class ENGINE_EXPORT RenderPass
	{
	private:
		static RenderPass* s_head;
		static RenderPass* s_tail;

		RenderPass* m_next = nullptr;
		Name m_name;

	protected:
		inline RenderPass* static_instance() { return nullptr; }

	public:
		RenderPass(const char* name);
		trinex_non_copyable(RenderPass);
		trinex_non_moveable(RenderPass);

		static RenderPass* static_find(const Name& name);

		virtual bool is_material_compatible(const Material* material);
		virtual RenderPass& modify_shader_compilation_env(ShaderCompilationEnvironment* env);
		virtual RenderPass* super_pass();

		inline const Name& name() const { return m_name; }
		inline RenderPass* next_pass() { return m_next; }

		static inline RenderPass* static_first_pass() { return s_head; }
		static inline RenderPass* static_last_pass() { return s_tail; }
		virtual ~RenderPass();
	};

#define trinex_render_pass(pass_name, parent)                                                                                    \
public:                                                                                                                          \
	using This  = pass_name;                                                                                                     \
	using Super = parent;                                                                                                        \
	static pass_name* static_instance();                                                                                         \
	pass_name(const char* name = #pass_name) : parent(name) {}                                                                   \
	parent* super_pass() override;                                                                                               \
                                                                                                                                 \
private:

#define trinex_implement_render_pass(pass_name)                                                                                  \
	pass_name* pass_name::static_instance()                                                                                      \
	{                                                                                                                            \
		static pass_name s_instance(#pass_name);                                                                                 \
		return &s_instance;                                                                                                      \
	}                                                                                                                            \
	pass_name::Super* pass_name::super_pass()                                                                                    \
	{                                                                                                                            \
		return Super::static_instance();                                                                                         \
	}                                                                                                                            \
	static const byte TRINEX_CONCAT(trinex_engine_refl_render_pass_, __LINE__) =                                                 \
	        Engine::ReflectionInitializeController([]() { pass_name::static_instance(); }).id()


	namespace RenderPasses
	{
		// Generic render pass with only depth buffer
		class ENGINE_EXPORT Depth : public RenderPass
		{
			trinex_render_pass(Depth, RenderPass);
		};

		// Generic render pass with one color attachment
		class ENGINE_EXPORT GenericOutput : public RenderPass
		{
			trinex_render_pass(GenericOutput, RenderPass);
		};

		// Generic render pass with four color attachments
		class ENGINE_EXPORT GenericGeometry : public RenderPass
		{
			trinex_render_pass(GenericGeometry, RenderPass);
			GenericGeometry& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;
		};
	}// namespace RenderPasses
}// namespace Engine
