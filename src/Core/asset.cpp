#include <Core/archive.hpp>
#include <Core/asset.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>

namespace Trinex
{
	Asset::Asset() : m_uuid(UUID::generate()) {}

	const UUID& Asset::uuid() const
	{
		return m_uuid;
	}

	UUID& Asset::uuid()
	{
		return m_uuid;
	}

	String Asset::uuid_string() const
	{
		return m_uuid.to_string();
	}

	bool Asset::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		return archive.serialize(m_uuid);
	}

	trinex_implement_engine_class(Asset, Refl::Class::IsAsset | Refl::Class::IsScriptable)
	{
		Refl::Property::create("UUID", &This::uuid_string, This::static_reflection(), Refl::Property::IsReadOnly);
	}
}// namespace Trinex
