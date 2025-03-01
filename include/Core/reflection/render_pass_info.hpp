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
		static RenderPassInfo* s_head;
		static RenderPassInfo* s_tail;

		Vector<ShaderDefinition> m_shader_definitions;
		Function<bool(const Material*)> m_is_material_compatible;
		RenderPassInfo* m_next     = nullptr;
		uint_t m_attachments_count = 1;

	public:
		static RenderPassInfo* static_find_pass(Name name);

		RenderPassInfo(Struct* parent = nullptr, BitMask flags = 0);
		~RenderPassInfo();
		RenderPassInfo& initialize() override;

		const Vector<ShaderDefinition>& shader_definitions() const;
		uint_t attachment_count() const;
		bool is_material_compatible(const Material* material);

		inline RenderPassInfo* next_pass()
		{
			return m_next;
		}

		static inline RenderPassInfo* first_pass()
		{
			return s_head;
		}

		static inline RenderPassInfo* last_pass()
		{
			return s_tail;
		}
	};


#define trinex_impl_render_pass(decl)                                                                                            \
	namespace                                                                                                                    \
	{                                                                                                                            \
		class TRINEX_CONCAT(RenderPass, __LINE__) : public Engine::Refl::RenderPassInfo                                          \
		{                                                                                                                        \
		public:                                                                                                                  \
			using This  = TRINEX_CONCAT(RenderPass, __LINE__);                                                                   \
			using Super = Engine::Refl::RenderPassInfo;                                                                          \
			using RenderPassInfo::RenderPassInfo;                                                                                \
			void initialize_render_pass();                                                                                       \
																																 \
			This& initialize() override                                                                                          \
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
