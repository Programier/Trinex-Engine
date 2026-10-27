#pragma once

#include <Core/deprecated/object.hpp>
#include <Core/types/uuid.hpp>

namespace Trinex
{
	class trinex_api Asset : public DeprecatedObject
	{
		trinex_class(Asset, DeprecatedObject);

	private:
		UUID m_uuid;
		String uuid_string() const;

	public:
		Asset();
		trinex_non_copyable(Asset);
		trinex_non_moveable(Asset);

		const UUID& uuid() const;
		UUID& uuid();

		bool serialize(Archive& archive) override;
	};
} // namespace Trinex
