#pragma once

#include <Core/object.hpp>
#include <Core/types/uuid.hpp>

namespace Trinex
{
	class ENGINE_EXPORT Asset : public Object
	{
		trinex_class(Asset, Object);

	private:
		UUID m_uuid;
		String uuid_string() const;

	public:
		delete_copy_constructors(Asset);
		Asset();

		const UUID& uuid() const;
		UUID& uuid();

		bool serialize(Archive& archive) override;
	};
}// namespace Trinex
