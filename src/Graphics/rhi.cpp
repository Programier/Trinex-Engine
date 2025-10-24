#include <Core/etl/allocator.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	ENGINE_EXPORT RHI* rhi = nullptr;

	RHIContext* RHI::context()
	{
		static RHIContext* ctx = []() {
			auto result = rhi->create_context();
			result->begin();
			return result;
		}();
		return ctx;
	}
}// namespace Engine
