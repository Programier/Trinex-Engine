#pragma once
#include <Core/reflection/enum.hpp>

namespace Trinex
{
	class Script;
	class ScriptTypeInfo;
}// namespace Trinex

namespace Trinex::Refl
{
	class ENGINE_EXPORT ScriptEnum : public Enum
	{
		trinex_reflect_type(ScriptEnum, Enum);

		Script* m_script;

	public:
		ScriptEnum(Script* script, const ScriptTypeInfo& info);
		Script* script() const;
		~ScriptEnum();
	};
}// namespace Trinex::Refl
