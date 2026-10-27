#pragma once
#include <Core/reflection/deprecated/class.hpp>
#include <ScriptEngine/script_function.hpp>

namespace Trinex
{
	class Script;
	class ScriptTypeInfo;
} // namespace Trinex

namespace Trinex::Refl::Deprecated
{
	class trinex_api ScriptClass : public Class
	{
		trinex_reflect_type(ScriptClass, Class);

	private:
		Script* m_script;
		ScriptFunction m_factory;

		Trinex::DeprecatedObject* object_constructor(StringView name, Trinex::DeprecatedObject* owner, bool scriptable) override;

	public:
		ScriptClass(Class* parent, Script* script, const ScriptTypeInfo& info, Struct::Flags flags = 0);
		ScriptClass& destroy_object(Trinex::DeprecatedObject* object) override;
		Script* script() const;
		usize size() const override;
		~ScriptClass();
	};
} // namespace Trinex::Refl::Deprecated
