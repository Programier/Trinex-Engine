#include <Core/exception.hpp>
#include <Core/threading.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	ENGINE_EXPORT RHI* rhi = nullptr;

	RHI_Object::RHI_Object(size_t init_ref_count) : m_references(init_ref_count) {}

	void RHI_Object::static_release_internal(RHI_Object* object)
	{
		if (is_in_render_thread())
		{
			object->release();
		}
		else
		{
			render_thread()->call([object]() { object->release(); });
		}
	}

	void RHI_Object::add_reference() const
	{
		++m_references;
	}

	void RHI_Object::release() const
	{
		if (m_references > 0)
			--m_references;

		if (m_references == 0)
		{
			destroy();
		}
	}

	size_t RHI_Object::references() const
	{
		return m_references;
	}

	RHI_Object::~RHI_Object() {}

	void RHI_Texture2D::clear_color(const Color& color)
	{
		throw EngineException("Surface only method is called on non-surface object!");
	}

	void RHI_Texture2D::clear_depth_stencil(float depth, byte stencil)
	{
		throw EngineException("Surface only method is called on non-surface object!");
	}

	void RHI_Texture2D::blit(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter)
	{
		throw EngineException("Surface only method is called on non-surface object!");
	}
}// namespace Engine
