#include <Core/userdata.hpp>

namespace Engine
{
	UserData& UserData::clear()
	{
		m_userdata.clear();
		return *this;
	}

	UserData& UserData::remove(Identifier id)
	{
		m_userdata.erase(id);
		return *this;
	}

	UserData& UserData::remove(const void* id)
	{
		return remove(reinterpret_cast<Identifier>(id));
	}

	Any& UserData::get(Identifier id)
	{
		return m_userdata[id];
	}

	Any& UserData::get(const void* id)
	{
		return get(reinterpret_cast<Identifier>(id));
	}
}// namespace Engine
