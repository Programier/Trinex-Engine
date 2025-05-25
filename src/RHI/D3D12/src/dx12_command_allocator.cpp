#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <dx12_api.hpp>
#include <dx12_command_allocator.hpp>

namespace Engine
{
	D3D12CommandAlloctor::Entry::Entry() : m_next(nullptr), m_last_signaled(0)
	{
		auto device = D3D12::api()->device();

		if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator))))
			throw EngineException("Failed to create command allocator");

		if (FAILED(device->CreateFence(m_last_signaled, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
			throw EngineException("Failed to create fence");
	}

	D3D12CommandAlloctor::D3D12CommandAlloctor()
	{
		m_current  = allocate<Entry>();
		m_first    = allocate<Entry>();
		m_push_ptr = &m_first->m_next;
	}

	D3D12CommandAlloctor::~D3D12CommandAlloctor()
	{
		Entry* next = nullptr;
		while (m_first)
		{
			next = m_first->m_next;
			release(m_first);
			m_first = next;
		}
	}

	D3D12CommandAlloctor::Entry* D3D12CommandAlloctor::current_entry()
	{
		if (m_current)
			return m_current;

		{
			auto value = m_first->m_fence->GetCompletedValue();

			if (value == m_first->m_last_signaled)
			{
				m_current = m_first;

				for (RHI_Object* object : m_current->m_pending_destroy)
				{
					release(object);
				}

				m_current->m_pending_destroy.clear();
				m_first = m_first->m_next;
				return m_current;
			}
		}

		m_current = allocate<Entry>();
		return m_current;
	}

	D3D12CommandAlloctor& D3D12CommandAlloctor::submit()
	{
		if (m_current)
		{
			auto queue = D3D12::api()->command_queue();
			queue->Signal(m_current->m_fence.Get(), ++m_current->m_last_signaled);

			m_current->m_next = nullptr;
			(*m_push_ptr)     = m_current;
			m_push_ptr        = &m_current->m_next;
			m_current         = nullptr;
		}
		return *this;
	}
}// namespace Engine
