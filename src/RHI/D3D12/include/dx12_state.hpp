#pragma once
#include <Core/etl/vector.hpp>
#include <dx12_buffer.hpp>
#include <dx12_headers.hpp>
#include <dx12_resource_view.hpp>

namespace Engine
{
	class D3D12_GraphicsPipeline;

	template<typename T, size_t count>
	class D3D12_ResourceState
	{
	private:
		T* m_resources[count];
		uint64_t m_dirty_flags;

		void add_ref(T* resource) {}
		void release(T* resource) {}

	public:
		D3D12_ResourceState& bind_resource(T* resource, byte index)
		{
			if (m_resources[index] != resource)
			{
				add_ref(resource);
				release(m_resources[index]);

				m_resources[index] = resource;
				m_dirty_flags |= (1 << index);
			}
			return *this;
		}

		bool is_resource_dirty(byte index) const { return (m_dirty_flags & (1 << index)) != 0; }
		bool is_any_resource_dirty() const { return m_dirty_flags != 0; }
		T* resource(byte index) const { return m_resources[index]; }
	};

	class D3D12_State
	{
	public:
		using SRVState           = D3D12_ResourceState<D3D12_SRV, RHI::s_max_srv>;
		using UAVState           = D3D12_ResourceState<D3D12_UAV, RHI::s_max_uav>;
		using RTVState           = D3D12_ResourceState<D3D12_RTV, 4>;
		using DSVState           = D3D12_ResourceState<D3D12_DSV, 1>;
		using PipelineState      = D3D12_ResourceState<D3D12_GraphicsPipeline, 1>;

	private:
		PipelineState m_pipeline;

		SRVState m_srv;
		UAVState m_uav;
		RTVState m_rtv;
		DSVState m_depth_stencil;

	private:
		inline bool is_render_target_dirty() const
		{
			return m_rtv.is_any_resource_dirty() || m_depth_stencil.is_any_resource_dirty();
		}

		D3D12_State& flush_render_target();

	public:
		inline const PipelineState& pipeline_state() const { return m_pipeline; }
		inline const SRVState& srv_state() const { return m_srv; }
		inline const UAVState& uav_state() const { return m_uav; }
		inline const RTVState& rtv_state() const { return m_rtv; }
		inline const DSVState& dsv_state() const { return m_depth_stencil; }

		inline D3D12_State& bind_srv(D3D12_SRV* srv, byte index)
		{
			m_srv.bind_resource(srv, index);
			return *this;
		}

		inline D3D12_State& bind_uav(D3D12_UAV* uav, byte index)
		{
			m_uav.bind_resource(uav, index);
			return *this;
		}

		inline D3D12_State& bind_pipeline(D3D12_GraphicsPipeline* pipeline)
		{
			m_pipeline.bind_resource(pipeline, 0);
			return *this;
		}

		inline D3D12_State& bind_render_target(D3D12_RTV* rt1 = nullptr, D3D12_RTV* rt2 = nullptr, D3D12_RTV* rt3 = nullptr,
		                                       D3D12_RTV* rt4 = nullptr, D3D12_DSV* depth_stencil = nullptr)
		{
			m_rtv.bind_resource(rt1, 0);
			m_rtv.bind_resource(rt2, 1);
			m_rtv.bind_resource(rt3, 2);
			m_rtv.bind_resource(rt4, 3);
			m_depth_stencil.bind_resource(depth_stencil, 0);
			return *this;
		}

		bool flush_graphics();
		bool flush_compute();
	};
}// namespace Engine
