#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class HelpMarker : public Element
	{
		trinex_ui_element(HelpMarker, Element);

	public:
		String text;

		UpdateFlags on_begin_update() override;
	};

	class Tooltip : public Element
	{
		trinex_ui_element(Tooltip, Element);

	public:
		String text;
		f32 delay = 0.0f;

		UpdateFlags on_begin_update() override;
	};

	class Badge : public Element
	{
		trinex_ui_element(Badge, Element);

	public:
		String text;
		Vec4 color = Vec4(0.0f, 0.0f, 0.0f, 0.0f);

		UpdateFlags on_begin_update() override;
	};

	class Pill : public Element
	{
		trinex_ui_element(Pill, Element);

	public:
		String text;
		Vec4 color = Vec4(0.0f, 0.0f, 0.0f, 0.0f);

		UpdateFlags on_begin_update() override;
	};

	class StatusDot : public Element
	{
		trinex_ui_element(StatusDot, Element);

	public:
		Vec4 color = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
		Unit radius = Unit(4.0f);

		UpdateFlags on_begin_update() override;
	};

	class KeyValueRow : public Element
	{
		trinex_ui_element(KeyValueRow, Element);

	public:
		String key;
		String value;

		UpdateFlags on_begin_update() override;
	};

	class Callout : public Element
	{
		trinex_ui_element(Callout, Element);

	public:
		String title;
		String message;

		UpdateFlags on_begin_update() override;
	};

	class Banner : public Element
	{
		trinex_ui_element(Banner, Element);

	public:
		String title;
		String message;
		Vec4 accent = Vec4(0.0f, 0.0f, 0.0f, 0.0f);

		UpdateFlags on_begin_update() override;
	};
}// namespace Trinex::UI
