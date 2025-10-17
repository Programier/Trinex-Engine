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

	RHIBuffer* RHI::create_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, size_t size,
	                                             RHIBufferCreateFlags flags)
	{
		StackByteAllocator::Mark mark;
		const byte* data = translate_ray_tracing_instances(instances, size);

		flags |= RHIBufferCreateFlags::AccelerationInput;
		flags |= RHIBufferCreateFlags::DeviceAddress;

		return create_buffer(size, data, flags);
	}
}// namespace Engine
