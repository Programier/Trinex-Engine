#pragma once

#include <Core/etl/set.hpp>
#include <RmlUi/Core/EventListener.h>
#include <UI/rml.hpp>

namespace Trinex
{
	class Package;
	class RHITexture;
}// namespace Trinex

namespace Trinex::UI
{
	class ContentBrowserController final : public RMLController, public RML::EventListener
	{
		trinex_class(ContentBrowserController, RMLController);

	private:
		RML::Element* m_element     = nullptr;
		RML::Element* m_tree        = nullptr;
		RML::Element* m_breadcrumbs = nullptr;
		RML::Element* m_grid_scroll = nullptr;
		RML::Element* m_grid        = nullptr;
		Package* m_selected_package = nullptr;
		Set<Package*> m_expanded_packages;
		bool m_dirty = true;

		ContentBrowserController& rebuild();
		ContentBrowserController& rebuild_tree();
		ContentBrowserController& rebuild_breadcrumbs();
		ContentBrowserController& rebuild_grid();
		ContentBrowserController& expand_package_path(Package* package);
		ContentBrowserController& append_package_tree(Package* package, usize depth);

		RML::Element* create_element(StringView tag, StringView class_names = {}) const;
		RML::Element* create_text(StringView text) const;
		RML::Element* create_img(StringView src, StringView width, StringView height) const;
		bool has_child_packages(Package* package) const;
		RML::Element* action_element_from_target(RML::Element* element) const;
		Package* package_from_element(RML::Element* element) const;

		RHITexture* load_thumb(Object* object);

	public:
		void ProcessEvent(RML::Event& event) override;

		ContentBrowserController& attach(RML::Element* element) override;
		ContentBrowserController& update(RML::Element* element) override;
		ContentBrowserController& deattach(RML::Element* element) override;
	};
}// namespace Trinex::UI
