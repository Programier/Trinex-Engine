#pragma once
#include <Core/reflection/object.hpp>

namespace Engine::Refl
{
	class ENGINE_EXPORT ScopedType : public Object
	{
		declare_reflect_type(ScopedType, Object);

		Map<StringView, Object*> m_childs;

		ScopedType& unregister_subobject(Object* subobject) override;
		ScopedType& register_subobject(Object* subobject) override;

	public:
		Object* find(StringView name, FindFlags flags = FindFlags::None) override;
		const Map<StringView, Object*>& childs() const;
	};
}// namespace Engine::Refl
