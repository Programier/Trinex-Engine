#include <UI/api.hpp>
#include <UI/backend.hpp>
#include <imgui.h>

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
		return nullptr;
	}

	RHITexture* pop_layer()
	{
		return nullptr;
	}
}// namespace Trinex::UI::Rendering
