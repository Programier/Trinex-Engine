#include <Core/etl/allocator.hpp>
#include <Graphics/render_pools.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_raytracing.hpp>

namespace Engine
{
	static inline vk::AccelerationStructureTypeKHR acceleration_type_of(RHIRayTracingAccelerationLevel level)
	{
		if (level == RHIRayTracingAccelerationLevel::Top)
			return vk::AccelerationStructureTypeKHR::eTopLevel;
		return vk::AccelerationStructureTypeKHR::eBottomLevel;
	}

	static inline vk::BuildAccelerationStructureFlagsKHR acceleration_flags_of(RHIRayTracingAccelerationFlags flags)
	{
		vk::BuildAccelerationStructureFlagsKHR result;

		if (flags & RHIRayTracingAccelerationFlags::AllowUpdate)
			result |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;

		if (flags & RHIRayTracingAccelerationFlags::AllowCompaction)
			result |= vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;

		if (flags & RHIRayTracingAccelerationFlags::PreferFastTrace)
			result |= vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;

		if (flags & RHIRayTracingAccelerationFlags::PreferFastBuild)
			result |= vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild;

		if (flags & RHIRayTracingAccelerationFlags::MinimizeMemory)
			result |= vk::BuildAccelerationStructureFlagBitsKHR::eLowMemory;

		return result;
	}

	static inline vk::BuildAccelerationStructureModeKHR acceleration_mode_of(RHIRayTracingAccelerationFlags flags)
	{
		if (flags & RHIRayTracingAccelerationFlags::PerformUpdate)
			return vk::BuildAccelerationStructureModeKHR::eUpdate;
		return vk::BuildAccelerationStructureModeKHR::eBuild;
	}

	static inline vk::GeometryTypeKHR geometry_type_of(RHIRayTracingGeometryType type)
	{
		switch (type)
		{
			case RHIRayTracingGeometryType::AABBs: return vk::GeometryTypeKHR::eAabbs;
			default: return vk::GeometryTypeKHR::eTriangles;
		}
	}

	static inline vk::GeometryFlagsKHR geometry_flags_of(RHIRayTracingGeometryFlags flags)
	{
		vk::GeometryFlagsKHR result;

		if (flags & RHIRayTracingGeometryFlags::Opaque)
			result |= vk::GeometryFlagBitsKHR::eOpaque;

		if (flags & RHIRayTracingGeometryFlags::NoDuplicateAnyHit)
			result |= vk::GeometryFlagBitsKHR::eNoDuplicateAnyHitInvocation;

		return result;
	}

	static inline vk::AccelerationStructureGeometryTrianglesDataKHR triangles_of(const RHIRayTracingGeometryTriangles* triangles)
	{
		vk::AccelerationStructureGeometryTrianglesDataKHR result;
		result.transformData = triangles->transform;
		result.vertexData    = triangles->vertex_buffer;
		result.indexData     = triangles->index_buffer;

		result.vertexStride = triangles->vertex_stride;
		result.maxVertex    = triangles->vertex_count;

		result.vertexFormat = VulkanEnums::vertex_format_of(triangles->vertex_format);
		if (triangles->index_buffer)
		{
			if (triangles->index_format == RHIIndexFormat::UInt16)
			{
				result.indexType = vk::IndexType::eUint16;
			}
			else
			{
				result.indexType = vk::IndexType::eUint32;
			}
		}
		else
		{
			result.indexType = vk::IndexType::eNoneKHR;
		}

		return result;
	}

	static inline vk::AccelerationStructureGeometryAabbsDataKHR aabbs_of(const RHIRayTracingGeometryAABBs* aabbs)
	{
		vk::AccelerationStructureGeometryAabbsDataKHR result;
		result.data   = aabbs->aabbs;
		result.stride = aabbs->stride;
		return result;
	}

	static inline vk::AccelerationStructureGeometryKHR geometry_of(const RHIRayTracingGeometry* geometry)
	{
		vk::AccelerationStructureGeometryKHR result;

		result.geometryType = geometry_type_of(geometry->type);
		result.flags        = geometry_flags_of(geometry->flags);

		if (geometry->type == RHIRayTracingGeometryType::Triangles)
		{
			result.geometry.triangles = triangles_of(geometry->triangles);
		}
		else
		{
			result.geometry.aabbs = aabbs_of(geometry->aabbs);
		}
		return result;
	}

	static inline vk::AccelerationStructureGeometryKHR geometry_of(vk::DeviceAddress address)
	{
		vk::AccelerationStructureGeometryKHR result;

		result.geometryType                       = vk::GeometryTypeKHR::eInstances;
		result.geometry.instances.arrayOfPointers = vk::False;
		result.geometry.instances.data            = address;
		return result;
	}

	static inline uint32_t primitives_count(const RHIRayTracingGeometryTriangles* triangles,
	                                        vk::AccelerationStructureBuildRangeInfoKHR* ranges)
	{
		const bool has_indices        = triangles->index_count && triangles->index_buffer;
		ranges->firstVertex           = 0;
		ranges->primitiveOffset       = 0;
		ranges->transformOffset       = 0;
		return ranges->primitiveCount = has_indices ? triangles->index_count : triangles->vertex_count;
	}

	static inline uint32_t primitives_count(const RHIRayTracingGeometryAABBs* aabbs,
	                                        vk::AccelerationStructureBuildRangeInfoKHR* ranges)
	{
		ranges->firstVertex           = 0;
		ranges->primitiveOffset       = 0;
		ranges->transformOffset       = 0;
		return ranges->primitiveCount = aabbs->count;
	}

	static inline uint32_t primitives_count(const RHIRayTracingAccelerationInputs* inputs,
	                                        vk::AccelerationStructureBuildRangeInfoKHR* ranges)
	{
		if (inputs->level == RHIRayTracingAccelerationLevel::Top)
		{
			ranges->firstVertex           = 0;
			ranges->primitiveOffset       = 0;
			ranges->transformOffset       = 0;
			return ranges->primitiveCount = inputs->count;
		}
		else
		{
			uint32_t primitives = 0;

			for (uint64_t i = 0; i < inputs->count; ++i, ++ranges)
			{
				if (inputs->geometries[i].type == RHIRayTracingGeometryType::Triangles)
					primitives += primitives_count(inputs->geometries[i].triangles, ranges);
				else
					primitives += primitives_count(inputs->geometries[i].aabbs, ranges);
			}

			return primitives;
		}
	}

	static inline vk::AccelerationStructureBuildGeometryInfoKHR geometry_info_of(const RHIRayTracingAccelerationInputs* inputs)
	{
		vk::AccelerationStructureBuildGeometryInfoKHR result;
		result.type  = acceleration_type_of(inputs->level);
		result.flags = acceleration_flags_of(inputs->flags);
		result.mode  = acceleration_mode_of(inputs->flags);

		if (inputs->level == RHIRayTracingAccelerationLevel::Top)
		{
			auto* geometries = StackAllocator<vk::AccelerationStructureGeometryKHR>::allocate(1);
			(*geometries)    = geometry_of(inputs->instances);

			result.geometryCount = 1;
			result.pGeometries   = geometries;
		}
		else
		{
			auto* geometries = StackAllocator<vk::AccelerationStructureGeometryKHR>::allocate(inputs->count);

			for (uint64_t i = 0; i < inputs->count; ++i)
			{
				geometries[i] = geometry_of(inputs->geometries + i);
			}

			result.geometryCount = 1;
			result.pGeometries   = geometries;
		}

		return result;
	}

	VulkanAccelerationStructure::VulkanAccelerationStructure(const RHIRayTracingAccelerationInputs* inputs)
	{
		StackByteAllocator::Mark mark;

		auto build_info = geometry_info_of(inputs);

		auto* build_ranges = StackAllocator<vk::AccelerationStructureBuildRangeInfoKHR>::allocate(build_info.geometryCount);
		const uint32_t primitives = primitives_count(inputs, build_ranges);

		constexpr auto build_type = vk::AccelerationStructureBuildTypeKHR::eDevice;
		auto sizes = API->m_device.getAccelerationStructureBuildSizesKHR(build_type, build_info, primitives, API->pfn);

		constexpr auto flags = RHIBufferCreateFlags::AccelerationStorage | RHIBufferCreateFlags::ByteAddressBuffer |
		                       RHIBufferCreateFlags::ShaderResource | RHIBufferCreateFlags::UnorderedAccess |
		                       RHIBufferCreateFlags::DeviceAddress;

		m_acceleration_buffer = trx_new VulkanBuffer();
		m_acceleration_buffer->create(sizes.accelerationStructureSize, nullptr, flags);

		vk::AccelerationStructureCreateInfoKHR info;
		info.buffer = m_acceleration_buffer->buffer();
		info.size   = m_acceleration_buffer->size();
		info.type   = build_info.type;
		info.offset = 0;

		m_acceleration = API->m_device.createAccelerationStructureKHR(info, nullptr, API->pfn);

		RHIBuffer* scratch = RHIBufferPool::global_instance()->request_buffer(sizes.buildScratchSize, flags);

		build_info.dstAccelerationStructure = m_acceleration;
		build_info.scratchData              = scratch->address();

		VulkanCommandHandle* cmd = API->current_command_buffer();
		cmd->buildAccelerationStructuresKHR(build_info, build_ranges, API->pfn);

		API->submit();
		RHIBufferPool::global_instance()->return_buffer(scratch);
	}

	VulkanAccelerationStructure::~VulkanAccelerationStructure()
	{
		API->m_device.destroyAccelerationStructureKHR(m_acceleration, nullptr, API->pfn);
		trx_delete m_acceleration_buffer;
	}

	RHIAccelerationStructure* VulkanAPI::create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs)
	{
		return trx_new VulkanAccelerationStructure(inputs);
	}
}// namespace Engine
