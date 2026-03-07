#pragma once
#include <Core/reflection/struct.hpp>

namespace Trinex
{
	class Script;
	class ScriptTypeInfo;
}// namespace Trinex

namespace Trinex::Refl
{
	class ENGINE_EXPORT ScriptStruct : public Struct
	{
		trinex_reflect_type(ScriptStruct, Struct);

	private:
		Script* m_script;

	public:
		ScriptStruct(ScriptStruct* parent, Script* script, const ScriptTypeInfo& info, BitMask flags = 0);
		void* create_struct() override;
		ScriptStruct& destroy_struct(void* obj) override;
		Script* script() const;
		usize size() const override;
		~ScriptStruct();
	};
}// namespace Trinex::Refl
