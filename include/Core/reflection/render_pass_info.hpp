#pragma once
#include <Core/reflection/struct.hpp>
#include <Core/structures.hpp>

namespace Engine::Refl
{
	class ENGINE_EXPORT RenderPassInfo : public Struct
	{
		declare_reflect_type(RenderPassInfo, Struct);

	public:
		struct Info {
			Vector<ShaderDefinition> shader_definitions;
			String entry                   = "main";
			uint_t color_attachments_count = 0;
			bool has_depth                 = false;
			bool has_stencil               = false;
			bool has_depth_stencil         = false;
		};

	protected:
		Info m_info;

	public:
		RenderPassInfo(Struct* parent = nullptr, BitMask flags = 0);

		const Vector<ShaderDefinition>& shader_definitions() const;
		const String& entry() const;
		uint_t color_attachment_count() const;
		bool has_color_attachments() const;
		bool has_depth_attachment() const;
		bool has_stencil_attachment() const;
		bool has_depth_stencil_attachment() const;
	};

	template<typename T, void (*initializer)(RenderPassInfo::Info&) = nullptr>
	class TypedRenderPassInfo : public RenderPassInfo
	{
	public:
		TypedRenderPassInfo(Struct* parent = nullptr, BitMask flags = 0) : RenderPassInfo(parent, flags)
		{}

		TypedRenderPassInfo& initialize() override
		{
			RenderPassInfo::initialize();
			if constexpr (initializer)
			{
				initializer(m_info);
			}
			return *this;
		}
	};

#define trinex_impl_render_pass(decl)                                                                                            \
	static void TRINEX_CONCAT(trinex_engine_refl_render_pass_initializer_, __LINE__)(Engine::Refl::RenderPassInfo::Info&);       \
	class Engine::Refl::Struct* decl::m_static_struct = nullptr;                                                                 \
                                                                                                                                 \
    class Engine::Refl::Struct* decl::static_struct_instance()                                                                   \
    {                                                                                                                            \
        if (!m_static_struct)                                                                                                    \
        {                                                                                                                        \
            constexpr auto func = &TRINEX_CONCAT(trinex_engine_refl_render_pass_initializer_, __LINE__);                         \
            m_static_struct = Engine::Refl::NativeStruct<decl, Engine::Refl::TypedRenderPassInfo<decl, func>>::create(#decl, 0); \
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
    static void TRINEX_CONCAT(trinex_engine_refl_render_pass_initializer_, __LINE__)(Engine::Refl::RenderPassInfo::Info & info)
}// namespace Engine::Refl
