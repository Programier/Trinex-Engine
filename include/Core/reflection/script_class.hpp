#pragma once
#include <Core/reflection/class.hpp>
#include <ScriptEngine/script_function.hpp>

namespace Trinex
{
	class Script;
	class ScriptTypeInfo;
}// namespace Trinex

namespace Trinex::Refl
{
	class ENGINE_EXPORT ScriptClass : public Class
	{
		trinex_reflect_type(ScriptClass, Class);

	private:
		Script* m_script;
		ScriptFunction m_factory;

		Trinex::Object* object_constructor(StringView name, Trinex::Object* owner, bool scriptable) override;

	public:
		ScriptClass(Class* parent, Script* script, const ScriptTypeInfo& info, BitMask flags = 0);
		ScriptClass& destroy_object(Trinex::Object* object) override;
		Script* script() const;
		usize size() const override;
		~ScriptClass();
	};
}// namespace Trinex::Refl
