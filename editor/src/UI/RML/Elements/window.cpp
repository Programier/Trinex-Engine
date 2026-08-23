#include <RmlUi/Core.h>

#include <cmath>
#include <imgui.h>

namespace Trinex::UI::RML::Backend
{
	Rml::Rectanglei desktop_rect();
}

namespace Trinex::UI
{
	namespace
	{
		class WindowElement final : public Rml::Element
		{
			RMLUI_RTTI_DeclareWithParent(WindowElement, Rml::Element);

		private:
			bool m_rect_valid = false;
			Rml::Vector2f m_pos;
			Rml::Vector2f m_size;

			static bool different(float a, float b) { return std::fabs(a - b) > 0.5f; }

			void sync_property(Rml::PropertyId property, float value)
			{
				const Rml::Property* current = GetProperty(property);

				if (current == nullptr || current->unit != Rml::Unit::PX || different(current->Get<float>(), value))
					SetProperty(property, Rml::Property(value, Rml::Unit::PX));
			}

			void sync_rect()
			{
				if (!m_rect_valid)
					return;

				sync_property(Rml::PropertyId::Left, m_pos.x);
				sync_property(Rml::PropertyId::Top, m_pos.y);
				sync_property(Rml::PropertyId::Width, m_size.x);
				sync_property(Rml::PropertyId::Height, m_size.y);
			}

			Rml::String title() const
			{
				Rml::String title = GetAttribute<Rml::String>("title", "Window");

				if (!GetId().empty())
				{
					title += "###";
					title += GetId();
				}

				return title;
			}

		public:
			WindowElement(const Rml::String& tag) : Rml::Element(tag)
			{
				ForceLocalStackingContext();
				SetProperty(Rml::PropertyId::Position, Rml::Property(Rml::Style::Position::Absolute));
				SetProperty("overflow", "hidden");
				SetClipArea(Rml::BoxArea::Border);
			}

			void OnUpdate() override
			{
				sync_rect();
				Rml::Element::OnUpdate();
			}

			void OnRender() override
			{
				const Rml::String caption = title();

				ImGui::Begin(caption.c_str());

				ImVec2 pos  = ImGui::GetCursorScreenPos();
				ImVec2 size = ImGui::GetContentRegionAvail();

				if (size.x < 1.f)
					size.x = 1.f;
				if (size.y < 1.f)
					size.y = 1.f;

				const Rml::Vector2f context_origin = Rml::Vector2f(RML::Backend::desktop_rect().TopLeft());
				const Rml::Vector2f content_pos    = {pos.x - context_origin.x, pos.y - context_origin.y};

				m_pos        = content_pos;
				m_size       = {size.x, size.y};
				m_rect_valid = true;

				const Rml::Vector2f absolute_offset = GetAbsoluteOffset(Rml::BoxArea::Border);
				const Rml::Vector2f render_offset   = content_pos - absolute_offset;

				if (ImGuiViewport* viewport = ImGui::GetWindowViewport())
				{
					const Rml::Vector2f viewport_offset = {viewport->Pos.x - context_origin.x,
					                                       viewport->Pos.y - context_origin.y};
					Rml::GetRenderInterface()->Begin({viewport->Size.x, viewport->Size.y}, viewport_offset, render_offset);
				}
				else
				{
					Rml::GetRenderInterface()->Begin(Rml::Vector2f(GetContext()->GetDimensions()), {}, render_offset);
				}
			}

			void OnPostRender() override
			{
				Rml::GetRenderInterface()->End();

				ImGui::End();
			}
		};

		RMLUI_RTTI_Define(WindowElement)
	}// namespace

	trinex_on_init()
	{
		static Rml::ElementInstancerGeneric<WindowElement> instancer;
		Rml::Factory::RegisterElementInstancer("window", &instancer);
	}
}// namespace Trinex::UI
