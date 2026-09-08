#include <CommonPlatform/context.hpp>

namespace Trinex::Platform
{
	CommonContext::~CommonContext() = default;

	trinex_weak Context* Context::instance()
	{
		trinex_unreachable_msg("Platform-specific Context::instance() implementation is missing");
		return nullptr;
	}

}// namespace Trinex::Platform
