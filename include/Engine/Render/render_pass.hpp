#pragma once
#include <Core/types/name.hpp>

namespace Trinex
{
	class Material;
	class ShaderCompilationEnvironment;

	class RHIContext;
	class Renderer;
	struct RHIDepthStencilState;
	struct RHIBlendingState;
	struct RHIRasterizerState;

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

		static inline RenderPass* static_first_pass() { return s_head; }
		static inline RenderPass* static_last_pass() { return s_tail; }
		static RenderPass* static_find(const Name& name);

		virtual RenderPass& begin(Renderer* renderer, RHIContext* ctx);
		virtual bool depth_stencil_state(RHIDepthStencilState& state);
		virtual bool blending_state(RHIBlendingState& state);
		virtual bool rasterizer_state(RHIRasterizerState& state);
		virtual RenderPass& end(Renderer* renderer, RHIContext* ctx);

		virtual bool is_material_compatible(const Material* material);
		virtual RenderPass& modify_shader_compilation_env(ShaderCompilationEnvironment* env);
		virtual RenderPass* super_pass();
		virtual String full_name() const;

		inline const Name& name() const { return m_name; }
		inline RenderPass* next() const { return m_next; }

		virtual ~RenderPass();
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
	trinex_on_reflection_init()                                                                                                  \
	{                                                                                                                            \
		pass_name::static_instance();                                                                                            \
	}                                                                                                                            \
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

	namespace RenderPasses
	{
		// Generic render pass with only depth buffer
		// class ENGINE_EXPORT Depth : public RenderPass
		// {
		// 	trinex_render_pass(Depth, RenderPass);
		// 	bool is_material_compatible(const Material* material) override;
		// 	Depth& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;
		// };

		// Generic render pass with four color attachments
		class ENGINE_EXPORT Geometry : public RenderPass
		{
			trinex_render_pass(Geometry, RenderPass);

		public:
			bool is_material_compatible(const Material* material) override;
			Geometry& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;

			Geometry& begin(Renderer* renderer, RHIContext* ctx) override;
			bool blending_state(RHIBlendingState& state) override;
		};

		// class ENGINE_EXPORT Translucent : public RenderPass
		// {
		// 	trinex_render_pass(Translucent, RenderPass);

		// public:
		// 	bool is_material_compatible(const Material* material) override;
		// 	Translucent& modify_shader_compilation_env(ShaderCompilationEnvironment* env) override;

		// 	Translucent& begin(Renderer* renderer, RHIContext* ctx) override;
		// };
	}// namespace RenderPasses
}// namespace Trinex
