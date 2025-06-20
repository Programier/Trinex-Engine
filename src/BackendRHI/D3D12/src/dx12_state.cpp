#include <dx12_api.hpp>
#include <dx12_pipeline.hpp>
#include <dx12_state.hpp>

namespace Engine
{
	D3D12_State& D3D12_State::flush_render_target()
	{
		auto cmd_list = D3D12::api()->cmd_list();
		if (is_render_target_dirty())
		{
			size_t count                           = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE handles[5] = {};

			for (int i = 0; i < 4; ++i)
			{
				if (auto rtv = m_rtv.resource(i))
				{
					handles[i] = rtv->descriptor().cpu_handle();
					count      = i + 1;
				}
			}

			if (m_depth_stencil.resource(0))
			{
				handles[4] = m_depth_stencil.resource(0)->descriptor().cpu_handle();
				cmd_list->OMSetRenderTargets(count, handles, FALSE, &handles[4]);
			}
			else
			{
				cmd_list->OMSetRenderTargets(count, handles, FALSE, nullptr);
			}
		}

		return *this;
	}

	bool D3D12_State::flush_graphics()
	{
		if (m_pipeline.resource(0) == nullptr)
			return false;

		m_pipeline.resource(0)->flush(this);

		flush_render_target();
		return true;
	}

	bool D3D12_State::flush_compute()
	{
		return true;
	}
}// namespace Engine
