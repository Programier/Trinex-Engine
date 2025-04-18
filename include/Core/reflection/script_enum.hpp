#pragma once
#include <Core/reflection/enum.hpp>

namespace Engine
{
	class Script;
	class ScriptTypeInfo;
}// namespace Engine

namespace Engine::Refl
{
	class ENGINE_EXPORT ScriptEnum : public Enum
	{
		declare_reflect_type(ScriptEnum, Enum);

		Script* m_script;

	public:
		ScriptEnum(Script* script, const ScriptTypeInfo& info);
		Script* script() const;
		~ScriptEnum();
	};
}// namespace Engine::Refl
