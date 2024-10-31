#pragma once
#include <Core/reflection/class.hpp>

namespace Engine
{
	class Script;
}

namespace Engine::Refl
{
	class ENGINE_EXPORT ScriptClass : public Class
	{
		declare_reflect_type(ScriptClass, Class);

		Script* m_script;

		Engine::Object* object_constructor(Class* class_overload, StringView name, Engine::Object* owner) override;

	public:
		ScriptClass(Class* parent, Script* script, BitMask flags = 0);
		ScriptClass& destroy_object(Engine::Object* object) override;
		~ScriptClass();
	};
}// namespace Engine::Refl
