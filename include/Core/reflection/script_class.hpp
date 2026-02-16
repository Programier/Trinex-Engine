#pragma once
#include <Core/reflection/class.hpp>
#include <ScriptEngine/script_function.hpp>

namespace Engine
{
	class Script;
	class ScriptTypeInfo;
}// namespace Engine

namespace Engine::Refl
{
	class ENGINE_EXPORT ScriptClass : public Class
	{
		trinex_reflect_type(ScriptClass, Class);

	private:
		Script* m_script;
		ScriptFunction m_factory;

		Engine::Object* object_constructor(StringView name, Engine::Object* owner, bool scriptable) override;

	public:
		ScriptClass(Class* parent, Script* script, const ScriptTypeInfo& info, BitMask flags = 0);
		ScriptClass& destroy_object(Engine::Object* object) override;
		Script* script() const;
		size_t size() const override;
		~ScriptClass();
	};
}// namespace Engine::Refl
