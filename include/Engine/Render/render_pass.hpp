#pragma once
#include <Core/name.hpp>

namespace Engine
{
	class Material;
	class ShaderCompilationEnvironment;
	class RenderPassPermutation;

	class ENGINE_EXPORT RenderPass
	{
	private:
		static RenderPass* s_head;
		static RenderPass* s_tail;

		RenderPassPermutation* m_permutations = nullptr;
		RenderPass* m_next                    = nullptr;
		Name m_name;

	protected:
		inline RenderPass* static_instance() { return nullptr; }

		RenderPass(const char* name, RenderPass* owner);

	public:
		RenderPass(const char* name);
		trinex_non_copyable(RenderPass);
		trinex_non_moveable(RenderPass);

		static inline RenderPass* static_first_pass() { return s_head; }
		static inline RenderPass* static_last_pass() { return s_tail; }
		static RenderPass* static_find(const Name& name);

		virtual bool is_material_compatible(const Material* material);
		virtual RenderPass& modify_shader_compilation_env(ShaderCompilationEnvironment* env);
		virtual RenderPass* super_pass();
		virtual String full_name() const;

		RenderPassPermutation* find_permutation(const Name& name) const;

		inline const Name& name() const { return m_name; }
		inline RenderPass* next() const { return m_next; }
		inline RenderPassPermutation* permutations() const { return m_permutations; }

		virtual ~RenderPass();
	};

	class ENGINE_EXPORT RenderPassPermutation : public RenderPass
	{
	private:
		RenderPass* m_owner;

	public:
		RenderPassPermutation(const char* name, RenderPass* owner);
		RenderPassPermutation& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;
		String full_name() const override;
		inline RenderPass* owner() const { return m_owner; }
	};

#define trinex_render_pass(pass_name, parent)                                                                                    \
public:                                                                                                                          \
	using This  = pass_name;                                                                                                     \
	using Super = parent;                                                                                                        \
	static pass_name* static_instance();                                                                                         \
	pass_name(const char* name = #pass_name);                                                                                    \
	parent* super_pass() override;                                                                                               \
                                                                                                                                 \
private:

#define trinex_implement_render_pass(pass_name)                                                                                  \
	static const byte TRINEX_CONCAT(trinex_engine_refl_render_pass_, __LINE__) =                                                 \
	        Engine::ReflectionInitializeController([]() { pass_name::static_instance(); }).id();                                 \
                                                                                                                                 \
	pass_name* pass_name::static_instance()                                                                                      \
	{                                                                                                                            \
		static pass_name s_instance(#pass_name);                                                                                 \
		return &s_instance;                                                                                                      \
	}                                                                                                                            \
	pass_name::Super* pass_name::super_pass()                                                                                    \
	{                                                                                                                            \
		return Super::static_instance();                                                                                         \
	}                                                                                                                            \
	pass_name::pass_name(const char* name) : Super(name)

#define trinex_implement_abstract_render_pass(pass_name)                                                                         \
	pass_name* pass_name::static_instance()                                                                                      \
	{                                                                                                                            \
		return nullptr;                                                                                                          \
	}                                                                                                                            \
	pass_name::Super* pass_name::super_pass()                                                                                    \
	{                                                                                                                            \
		return Super::static_instance();                                                                                         \
	}                                                                                                                            \
	pass_name::pass_name(const char* name) : Super(name)

	namespace RenderPassPermutations
	{
		class ENGINE_EXPORT StaticMesh : public RenderPassPermutation
		{
		public:
			StaticMesh(RenderPass* owner) : RenderPassPermutation("StaticMesh", owner) {}
			StaticMesh& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;
		};

		class ENGINE_EXPORT SkeletalMesh : public RenderPassPermutation
		{
		public:
			SkeletalMesh(RenderPass* owner) : RenderPassPermutation("SkeletalMesh", owner) {}
			SkeletalMesh& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;
		};
	}// namespace RenderPassPermutations

	namespace RenderPasses
	{
		// Generic render pass with only depth buffer
		class ENGINE_EXPORT Depth : public RenderPass
		{
			trinex_render_pass(Depth, RenderPass);
			Depth& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;
		};

		// Generic render pass with four color attachments
		class ENGINE_EXPORT Geometry : public RenderPass
		{
			trinex_render_pass(Geometry, RenderPass);

		public:
			Geometry& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;
		};
	}// namespace RenderPasses
}// namespace Engine
