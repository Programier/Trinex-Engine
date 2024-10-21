#pragma once
#include <Core/reflection/object.hpp>

namespace Engine::Refl
{
	class ENGINE_EXPORT ScopedType : public Object
	{
		declare_reflect_type(ScopedType, Object);

		Map<String, Object*> m_childs;

		ScopedType& unregister_subobject(Object* subobject) override;
		ScopedType& register_subobject(Object* subobject) override;

		~ScopedType();
	public:
		Object* find(StringView name, FindFlags flags = FindFlags::None) override;
		const Map<String, Object*>& childs() const;
	};
}// namespace Engine::Refl
