#include <UI/api.hpp>
#include <UI/backend.hpp>

namespace Trinex::UI::Rendering
{
	RHIContext* context()
	{
		return UI::Backend::rhi();
	}

	RHITexture* layer()
	{
		return UI::Backend::layer();
	}

	RHITexture* push_layer()
	{
		return UI::Backend::push_layer();
	}

	RHITexture* pop_layer()
	{
		return UI::Backend::pop_layer();
	}
}// namespace Trinex::UI::Rendering
