#pragma once
#include <Core/reflection/struct.hpp>

namespace Engine
{
	class Script;
	class ScriptTypeInfo;
}

namespace Engine::Refl
{
	class ENGINE_EXPORT ScriptStruct : public Struct
	{
		declare_reflect_type(ScriptStruct, Struct);

	private:
		Script* m_script;

	public:
		ScriptStruct(ScriptStruct* parent, Script* script, const ScriptTypeInfo& info, BitMask flags = 0);
		void* create_struct() override;
		ScriptStruct& destroy_struct(void* obj) override;
		Script* script() const;
		size_t size() const override;
		~ScriptStruct();
	};
}// namespace Engine::Refl
