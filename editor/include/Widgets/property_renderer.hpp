#pragma once
#include <Graphics/imgui.hpp>

namespace Engine
{
	namespace Refl
	{
		class Struct;
	}// namespace Refl

	class ScriptFunction;

	class PropertyRenderer : public ImGuiWidget
	{
	private:
		void* m_object;
		Refl::Struct* m_struct;
		Identifier m_destroy_id;

	public:
	public:
		PropertyRenderer();
		~PropertyRenderer();

		bool render(RenderViewport* viewport) override;
		PropertyRenderer& render();

		PropertyRenderer& object(Object* object);
		PropertyRenderer& object(void* object, Refl::Struct* self);

		inline void* object() const { return m_object; }
		inline Refl::Struct* object_reflection() const { return m_struct; }

		virtual const char* name() const;
		static const char* static_name();
	};

}// namespace Engine
