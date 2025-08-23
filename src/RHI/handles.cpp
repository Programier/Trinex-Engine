#include <Core/math/math.hpp>
#include <Core/threading.hpp>
#include <RHI/handles.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	RHIObject::RHIObject(size_t init_ref_count) : m_references(init_ref_count) {}

	void RHIObject::static_release_internal(RHIObject* object)
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

	void RHIObject::add_reference()
	{
		++m_references;
	}

	void RHIObject::release()
	{
		if (m_references > 0)
			--m_references;

		if (m_references == 0)
		{
			destroy();
		}
	}

	size_t RHIObject::references() const
	{
		return m_references;
	}

	RHIObject::~RHIObject() {}

	RHITextureView::RHITextureView(RHITexture* texture, RHITextureType type, uint16_t base_slice, uint16_t slice_count,
	                               uint16_t base_mip, uint16_t mip_count)
	    : m_texture(texture), m_base_slice(base_slice), m_slice_count(slice_count), m_base_mip(base_mip), m_mip_count(mip_count)
	{}

	template<typename T>
	T RHITextureView::initialize_description(const T* desc)
	{
		T view = desc ? *desc : T();

		if (view.view_type == RHITextureType::Undefined)
			view.view_type = static_cast<RHITextureType::Enum>(m_type);

		view.base_slice += m_base_slice;
		view.base_mip += m_base_mip;
		view.slice_count = Math::min(m_slice_count, view.slice_count);
		return view;
	}

	void RHITextureView::destroy() {}

	RHIRenderTargetView* RHITextureView::as_rtv(RHITextureDescRTV* desc)
	{
		RHITextureDescRTV view = initialize_description(desc);
		return m_texture->as_rtv(&view);
	}

	RHIDepthStencilView* RHITextureView::as_dsv(RHITextureDescDSV* desc)
	{
		RHITextureDescDSV view = initialize_description(desc);
		return m_texture->as_dsv(&view);
	}

	RHIShaderResourceView* RHITextureView::as_srv(RHITextureDescSRV* desc)
	{
		RHITextureDescSRV view = initialize_description(desc);
		view.mip_count         = Math::min(m_mip_count, view.mip_count);
		return m_texture->as_srv(&view);
	}

	RHIUnorderedAccessView* RHITextureView::as_uav(RHITextureDescUAV* desc)
	{
		RHITextureDescUAV view = initialize_description(desc);
		return m_texture->as_uav(&view);
	}
}// namespace Engine
