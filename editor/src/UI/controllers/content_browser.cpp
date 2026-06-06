#include <Core/editor_resources.hpp>
#include <Core/math/math.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/texture.hpp>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <UI/controllers/content_browser.hpp>

namespace Trinex::UI
{
	namespace
	{
		static void collect_package_chain(Vector<Package*>& chain, Package* package)
		{
			if (package == nullptr)
				return;

			collect_package_chain(chain, package->package());
			chain.push_back(package);
		}
	}// namespace

	trinex_implement_class(Trinex::UI::ContentBrowserController, 0) {}

	RML::Element* ContentBrowserController::create_element(StringView tag, StringView class_names) const
	{
		if (m_element == nullptr)
			return nullptr;

		RML::ElementDocument* document = m_element->GetOwnerDocument();
		if (document == nullptr)
			return nullptr;

		RML::ElementPtr element = document->CreateElement(String(tag));
		if (element == nullptr)
			return nullptr;

		if (!class_names.empty())
		{
			element->SetClassNames(String(class_names));
		}

		return element.release();
	}

	RML::Element* ContentBrowserController::create_text(StringView text) const
	{
		if (m_element == nullptr)
			return nullptr;

		RML::ElementDocument* document = m_element->GetOwnerDocument();
		if (document == nullptr)
			return nullptr;

		RML::ElementPtr element = document->CreateTextNode(String(text));
		return element.release();
	}

	RML::Element* ContentBrowserController::create_img(StringView src, StringView width, StringView height) const
	{
		if (m_element == nullptr)
			return nullptr;

		RML::ElementDocument* document = m_element->GetOwnerDocument();
		if (document == nullptr)
			return nullptr;

		RML::ElementPtr element = document->CreateElement("img");
		if (element == nullptr)
			return nullptr;

		element->SetAttribute("src", String(src));
		element->SetProperty("width", String(width));
		element->SetProperty("height", String(height));
		return element.release();
	}

	bool ContentBrowserController::has_child_packages(Package* package) const
	{
		if (package == nullptr)
			return false;

		for (Object* object : package->objects())
		{
			if (object && object->instance_cast<Package>())
				return true;
		}

		return false;
	}

	usize ContentBrowserController::package_item_count(Package* package) const
	{
		return package ? package->objects().size() : 0;
	}

	RML::Element* ContentBrowserController::action_element_from_target(RML::Element* element) const
	{
		for (RML::Element* current = element; current && current != m_element; current = current->GetParentNode())
		{
			if (current->HasAttribute("data-action"))
				return current;
		}

		return nullptr;
	}

	Package* ContentBrowserController::package_from_element(RML::Element* element) const
	{
		if (RML::Element* current = action_element_from_target(element))
		{
			const String action = current->GetAttribute<String>("data-action", "");
			if (action == "select-package" || action == "toggle-package")
			{
				const String package_name = current->GetAttribute<String>("data-package", "");
				return package_name.empty() ? nullptr : Object::static_find_package(package_name, false);
			}
		}

		return nullptr;
	}

	RHITexture* ContentBrowserController::load_thumb(Object* object)
	{
		if (Texture2D* texture = object->instance_cast<Texture2D>())
		{
			return texture->rhi_texture();
		}

		return EditorResources::default_icon->rhi_texture();
	}

	ContentBrowserController& ContentBrowserController::expand_package_path(Package* package)
	{
		for (Package* current = package; current; current = current->package())
		{
			m_expanded_packages.insert(current);
		}

		return *this;
	}

	ContentBrowserController& ContentBrowserController::append_package_tree(RML::Element* parent, Package* package, usize depth)
	{
		if (package == nullptr || parent == nullptr)
			return *this;

		RML::Element* node = create_element("div", "trx-content-browser__tree-node");
		if (node == nullptr)
			return *this;

		const bool expandable = has_child_packages(package);
		const bool expanded   = m_expanded_packages.contains(package);

		if (depth == 0)
		{
			node->SetClass("trx-content-browser__tree-node--root", true);
		}

		RML::Element* entry = create_element("div", "trx-content-browser__tree-entry");
		if (entry == nullptr)
			return *this;

		if (package == m_selected_package)
		{
			entry->SetClass("trx-content-browser__tree-entry--active", true);
		}

		entry->SetAttribute("data-action", "select-package");
		entry->SetAttribute("data-package", package->full_name());
		entry->SetProperty("padding-left", Strings::format("{}px", 4 + depth * 18));

		RML::Element* expander = create_element("span", "trx-content-browser__tree-expander");
		if (expander != nullptr)
		{
			if (expandable)
			{
				expander->SetAttribute("data-action", "toggle-package");
				expander->SetAttribute("data-package", package->full_name());
				expander->AppendChild(RML::ElementPtr(create_text(expanded ? "v" : ">")));
			}
			else
			{
				expander->SetClass("trx-content-browser__tree-expander--empty", true);
			}

			entry->AppendChild(RML::ElementPtr(expander));
		}

		RML::Element* icon = create_element("span", "trx-content-browser__tree-icon");
		if (icon != nullptr)
		{
			icon->SetClass("trx-content-browser__tree-icon--folder", true);
			entry->AppendChild(RML::ElementPtr(icon));
		}

		RML::Element* label = create_element("span", "trx-content-browser__tree-label");
		if (label != nullptr)
		{
			label->AppendChild(RML::ElementPtr(create_text(package->string_name())));
			entry->AppendChild(RML::ElementPtr(label));
		}

		RML::Element* meta = create_element("span", "trx-content-browser__tree-meta");
		if (meta != nullptr)
		{
			meta->SetClass("trx-content-browser__tree-meta--count", true);
			meta->AppendChild(RML::ElementPtr(create_text(Strings::format("{}", package_item_count(package)))));
			entry->AppendChild(RML::ElementPtr(meta));
		}

		node->AppendChild(RML::ElementPtr(entry));

		if (!expandable || !expanded)
		{
			parent->AppendChild(RML::ElementPtr(node));
			return *this;
		}

		RML::Element* children = create_element("div", "trx-content-browser__tree-children");
		if (children != nullptr)
		{
			for (Object* object : package->objects())
			{
				if (Package* child = object ? object->instance_cast<Package>() : nullptr)
				{
					append_package_tree(children, child, depth + 1);
				}
			}

			node->AppendChild(RML::ElementPtr(children));
		}

		parent->AppendChild(RML::ElementPtr(node));

		return *this;
	}

	ContentBrowserController& ContentBrowserController::rebuild_tree()
	{
		if (m_tree == nullptr)
			return *this;

		m_tree->SetInnerRML("");
		append_package_tree(m_tree, Object::root_package(), 0);
		return *this;
	}

	ContentBrowserController& ContentBrowserController::rebuild_breadcrumbs()
	{
		if (m_breadcrumbs == nullptr)
			return *this;

		m_breadcrumbs->SetInnerRML("");

		Vector<Package*> chain;
		collect_package_chain(chain, m_selected_package);

		for (usize i = 0; i < chain.size(); ++i)
		{
			if (i > 0)
			{
				if (RML::Element* separator = create_element("span", "trx-content-browser__breadcrumb-separator"))
				{
					separator->AppendChild(RML::ElementPtr(create_text("/")));
					m_breadcrumbs->AppendChild(RML::ElementPtr(separator));
				}
			}

			Package* package    = chain[i];
			RML::Element* crumb = create_element("div", "trx-content-browser__breadcrumb");
			if (crumb == nullptr)
				continue;

			crumb->SetAttribute("data-action", "select-package");
			crumb->SetAttribute("data-package", package->full_name());

			if (package == m_selected_package)
			{
				crumb->SetClass("trx-content-browser__breadcrumb--active", true);
			}

			crumb->AppendChild(RML::ElementPtr(create_text(package->string_name())));
			m_breadcrumbs->AppendChild(RML::ElementPtr(crumb));
		}

		return *this;
	}

	ContentBrowserController& ContentBrowserController::rebuild_grid()
	{
		if (m_grid == nullptr)
			return *this;

		m_grid->SetInnerRML("");

		if (m_selected_package == nullptr)
			return *this;

		for (Object* object : m_selected_package->objects())
		{
			if (object == nullptr)
				continue;

			RML::Element* card = create_element("div", "trx-content-browser__item");
			if (card == nullptr)
				continue;

			if (Package* package = object->instance_cast<Package>())
			{
				card->SetClass("trx-content-browser__item--package", true);
				card->SetAttribute("data-action", "select-package");
				card->SetAttribute("data-package", package->full_name());
			}

			if (RML::Element* thumb = create_element("div", "trx-content-browser__thumb"))
			{
				if (RHITexture* texture = load_thumb(object))
				{
					String address = Strings::format("#{}", static_cast<void*>(texture));
					thumb->AppendChild(RML::ElementPtr(create_img(address, "100%", "100%")));
				}

				card->AppendChild(RML::ElementPtr(thumb));
			}

			if (RML::Element* name = create_element("div", "trx-content-browser__item-name"))
			{
				name->AppendChild(RML::ElementPtr(create_text(object->string_name())));
				card->AppendChild(RML::ElementPtr(name));
			}

			if (RML::Element* meta = create_element("div", "trx-content-browser__item-meta"))
			{
				meta->AppendChild(RML::ElementPtr(create_text(object->class_instance()->name())));
				card->AppendChild(RML::ElementPtr(meta));
			}

			m_grid->AppendChild(RML::ElementPtr(card));
		}

		return *this;
	}

	ContentBrowserController& ContentBrowserController::rebuild()
	{
		if (m_selected_package == nullptr)
		{
			m_selected_package = Object::root_package();
		}

		expand_package_path(m_selected_package);
		rebuild_tree();
		rebuild_breadcrumbs();
		rebuild_grid();
		m_dirty = false;
		return *this;
	}

	void ContentBrowserController::ProcessEvent(RML::Event& event)
	{
		if (m_element == nullptr || !(event == "click"))
			return;

		RML::Element* action_element = action_element_from_target(event.GetTargetElement());
		if (action_element == nullptr)
			return;

		const String action = action_element->GetAttribute<String>("data-action", "");
		if (Package* package = package_from_element(action_element))
		{
			if (action == "toggle-package")
			{
				if (m_expanded_packages.contains(package))
				{
					m_expanded_packages.erase(package);
				}
				else
				{
					m_expanded_packages.insert(package);
				}

				m_dirty = true;
			}
			else if (action == "select-package")
			{
				if (m_selected_package != package)
				{
					m_selected_package = package;
					expand_package_path(package);
					m_dirty = true;
				}
			}

			event.StopPropagation();
		}
	}

	ContentBrowserController& ContentBrowserController::attach(RML::Element* element)
	{
		Super::attach(element);

		m_element          = element;
		m_tree             = m_element ? m_element->GetElementById("trx-content-browser-tree") : nullptr;
		m_breadcrumbs      = m_element ? m_element->GetElementById("trx-content-browser-breadcrumbs") : nullptr;
		m_grid_scroll      = m_element ? m_element->GetElementById("trx-content-browser-grid-scroll") : nullptr;
		m_grid             = m_element ? m_element->GetElementById("trx-content-browser-grid") : nullptr;
		m_selected_package = Object::root_package();
		m_expanded_packages.clear();
		expand_package_path(m_selected_package);
		m_dirty = true;

		if (m_element)
		{
			m_element->AddEventListener("click", this);
		}

		return *this;
	}

	ContentBrowserController& ContentBrowserController::update(RML::Element* element)
	{
		Super::update(element);

		if (m_element != element)
			return *this;

		if (m_dirty)
		{
			rebuild();
		}
		return *this;
	}

	ContentBrowserController& ContentBrowserController::deattach(RML::Element* element)
	{
		if (m_element)
		{
			m_element->RemoveEventListener("click", this);
		}

		m_element          = nullptr;
		m_tree             = nullptr;
		m_breadcrumbs      = nullptr;
		m_grid_scroll      = nullptr;
		m_grid             = nullptr;
		m_selected_package = nullptr;
		m_expanded_packages.clear();
		m_dirty = true;

		Super::deattach(element);
		return *this;
	}
}// namespace Trinex::UI
