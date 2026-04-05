#pragma once
#include <Core/object_listener.hpp>
#include <UI/imgui.hpp>

namespace Trinex
{
	namespace Refl
	{
		class Struct;
	}// namespace Refl

	class ScriptFunction;

	class PropertyRenderer : public ImGuiWidget, public ObjectDestroyListener
	{
	private:
		void* m_object         = nullptr;
		Refl::Struct* m_struct = nullptr;

	public:
		PropertyRenderer& on_object_destroy(Object* object) override;
		bool render(RenderViewport* viewport) override;
		PropertyRenderer& render();

		PropertyRenderer& object(Object* object);
		PropertyRenderer& object(void* object, Refl::Struct* self);

		inline void* object() const { return m_object; }
		inline Refl::Struct* object_reflection() const { return m_struct; }

		virtual const char* name() const;
		static const char* static_name();
	};

}// namespace Trinex
