#include <Core/math/math.hpp>
#include <UI/Elements/visual.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	trinex_implement_ui_element(Visual)
	{
		trinex_ui_bind_property(pivot);
		trinex_ui_bind_property(translate);
		trinex_ui_bind_property(scale);
		trinex_ui_bind_property(rotate);
	}


	Visual& Visual::push_style()
	{
		Super::push_style();

		ImGui::BeginTransform({
		        .Translation = translate,
		        .Scale       = scale,
		        .Rotation    = Math::radians(rotate),
		        .Pivot       = pivot,
		});

		return *this;
	}

	Visual& Visual::pop_style()
	{
		ImGui::EndTransform();
		return *Super::pop_style().as<This>();
	}
}// namespace Trinex::UI
