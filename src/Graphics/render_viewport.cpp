#include <Core/etl/algorithm.hpp>
#include <Core/viewport_client.hpp>
#include <Graphics/render_viewport.hpp>
#include <RHI/handles.hpp>
#include <RHI/resource_ptr.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	Array<Ref<RenderViewport>> RenderViewport::s_viewports = {};

	RenderViewport::RenderViewport(Ref<Window> window, RHIPresentMode mode, u32 images)
	{
		trinex_verify(window);

		m_swapchain = RHI::instance()->create_swapchain(window.value(), mode, images);
		trinex_verify(m_swapchain);

		s_viewports.push_back(retain_ref(this));
	}

	RenderViewport::~RenderViewport()
	{
		auto it = etl::find(s_viewports.begin(), s_viewports.end(), this);

		if (it != s_viewports.end())
		{
			s_viewports.erase_unordered(it);
		}
	}

	RenderViewport& RenderViewport::client(Ref<ViewportClient> client)
	{
		if (m_client)
		{
			Ref<ViewportClient> tmp;
			tmp.swap(m_client);

			tmp->deattach(this);
		}

		m_client.swap(client);

		if (m_client)
		{
			m_client->attach(this);
		}

		return *this;
	}

	RenderViewport& RenderViewport::mode(RHIPresentMode mode)
	{
		if (m_swapchain)
		{
			m_swapchain->mode(mode);
		}
		return *this;
	}

	RHISemaphore* RenderViewport::acquire_semaphore()
	{
		return m_swapchain ? m_swapchain->acquire_semaphore() : nullptr;
	}

	RHISemaphore* RenderViewport::present_semaphore()
	{
		return m_swapchain ? m_swapchain->present_semaphore() : nullptr;
	}

	RHITexture* RenderViewport::backbuffer()
	{
		return m_swapchain ? m_swapchain->backbuffer() : nullptr;
	}

	void RenderViewport::update(float dt)
	{
		if (m_client)
		{
			m_client->update(this, dt);
		}
	}

	void RenderViewport::render(RenderGraph* graph)
	{
		if (m_client)
		{
			m_client->render(this, graph);
		}
	}

	void RenderViewport::present() {}
} // namespace Trinex
