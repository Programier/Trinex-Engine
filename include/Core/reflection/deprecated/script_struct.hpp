#pragma once
#include <Core/reflection/deprecated/struct.hpp>

namespace Trinex
{
	class Script;
	class ScriptTypeInfo;
} // namespace Trinex

namespace Trinex::Refl::Deprecated
{
	class trinex_api ScriptStruct : public Struct
	{
		trinex_reflect_type(ScriptStruct, Struct);

	private:
		Script* m_script;

	public:
		ScriptStruct(ScriptStruct* parent, Script* script, const ScriptTypeInfo& info, Struct::Flags flags = 0);
		void* create_struct() override;
		ScriptStruct& destroy_struct(void* obj) override;
		Script* script() const;
		usize size() const override;
		~ScriptStruct();
	};
} // namespace Trinex::Refl::Deprecated
