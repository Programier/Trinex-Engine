#pragma once
#include <Platform/platform.hpp>

namespace Trinex::Platform
{
	class trinex_api CommonContext : public Context
	{
	public:
		~CommonContext() override;
	};
}// namespace Trinex::Platform
