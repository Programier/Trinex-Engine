#pragma once
#include <Core/reflection/struct.hpp>
#include <Core/structures.hpp>

namespace Engine
{
	class Material;
}

namespace Engine::Refl
{
	class ENGINE_EXPORT RenderPassInfo : public Struct
	{
		declare_reflect_type(RenderPassInfo, Struct);

	protected:
		Vector<ShaderDefinition> m_shader_definitions;
		Function<bool(const Material*)> m_is_material_support;
		String m_entry                   = "main";
		uint_t m_color_attachments_count = 0;
		bool m_has_depth                 = false;
		bool m_has_stencil               = false;
		bool m_has_depth_stencil         = false;

	public:
		RenderPassInfo(Struct* parent = nullptr, BitMask flags = 0);

		const Vector<ShaderDefinition>& shader_definitions() const;
		const String& entry() const;
		uint_t color_attachment_count() const;
		bool has_color_attachments() const;
		bool has_depth_attachment() const;
		bool has_stencil_attachment() const;
		bool has_depth_stencil_attachment() const;
		bool is_material_support(const Material* material);
	};


#define trinex_impl_render_pass(decl)                                                                                            \
	namespace                                                                                                                    \
	{                                                                                                                            \
		class TRINEX_CONCAT(RenderPass, __LINE__) : public Engine::Refl::RenderPassInfo                                          \
		{                                                                                                                        \
		public:                                                                                                                  \
			using RenderPassInfo::RenderPassInfo;                                                                                \
			void initialize_render_pass();                                                                                       \
			Super& initialize() override                                                                                         \
			{                                                                                                                    \
				Super::initialize();                                                                                             \
				initialize_render_pass();                                                                                        \
				return *this;                                                                                                    \
			}                                                                                                                    \
		};                                                                                                                       \
	}                                                                                                                            \
	class Engine::Refl::Struct* decl::m_static_struct = nullptr;                                                                 \
                                                                                                                                 \
    class Engine::Refl::Struct* decl::static_struct_instance()                                                                   \
    {                                                                                                                            \
        if (!m_static_struct)                                                                                                    \
        {                                                                                                                        \
            m_static_struct = Engine::Refl::NativeStruct<decl, TRINEX_CONCAT(RenderPass, __LINE__)>::create(#decl, 0);           \
        }                                                                                                                        \
        return m_static_struct;                                                                                                  \
    }                                                                                                                            \
                                                                                                                                 \
    Engine::Refl::Struct* decl::struct_instance() const                                                                          \
    {                                                                                                                            \
        return static_struct_instance();                                                                                         \
    }                                                                                                                            \
                                                                                                                                 \
    static Engine::byte TRINEX_CONCAT(trinex_engine_refl_render_pass_, __LINE__) = static_cast<Engine::byte>(                    \
            Engine::Refl::Object::static_register_initializer([]() { decl::static_struct_instance(); }, #decl));                 \
                                                                                                                                 \
    void decl::static_initialize_struct()                                                                                        \
    {}                                                                                                                           \
                                                                                                                                 \
    void TRINEX_CONCAT(RenderPass, __LINE__)::initialize_render_pass()
}// namespace Engine::Refl
