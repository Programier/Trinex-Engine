#include <Core/garbage_collector.hpp>
#include <Core/reflection/class.hpp>
#include <UI/property_renderer.hpp>
#include <Widgets/property_renderer.hpp>

namespace Engine
{
	PropertyRenderer::PropertyRenderer() : m_object(nullptr), m_struct(nullptr)
	{
		m_destroy_id = GarbageCollector::on_destroy.push([this](Object* garbage) {
			if (m_object == garbage)
			{
				m_object = nullptr;
				m_struct = nullptr;
			}
		});
	}

	PropertyRenderer::~PropertyRenderer()
	{
		GarbageCollector::on_destroy.remove(m_destroy_id);
	}

	bool PropertyRenderer::render(RenderViewport* viewport)
	{
		bool open = true;
		ImGui::Begin(name(), closable ? &open : nullptr);
		render();
		ImGui::End();

		return open;
	}

	PropertyRenderer& PropertyRenderer::render()
	{
		if (m_object && m_struct)
		{
			auto renderer = UI::PropertyRenderer::static_renderer();
			renderer->begin();
			renderer->render_properties(m_object, m_struct, false);
			renderer->end();
		}
		return *this;
	}

	PropertyRenderer& PropertyRenderer::object(Object* object)
	{
		m_object = object;
		m_struct = object ? object->class_instance() : nullptr;
		return *this;
	}

	PropertyRenderer& PropertyRenderer::object(void* object, Refl::Struct* self)
	{
		m_object = (object && self) ? object : nullptr;
		m_struct = (object && self) ? self : nullptr;
		return *this;
	}

	const char* PropertyRenderer::name() const
	{
		return static_name();
	}

	const char* PropertyRenderer::static_name()
	{
		return "editor/Properties"_localized;
	}

}// namespace Engine
