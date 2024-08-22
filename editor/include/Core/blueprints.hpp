#pragma once
#include <Core/engine_types.hpp>
#include <imgui_node_editor.h>

namespace Engine
{
	namespace ed = ax::NodeEditor;

	class BlueprintBuilder
	{
	public:
		enum class IconType : ImU32
		{
			Flow,
			Circle,
			Square,
			Grid,
			RoundSquare,
			Diamond
		};

	private:
		enum class Stage
		{
			Invalid = 0,
			Begin   = 1,
			End     = 2,
			Header  = 3,
			Content = 4,
			Input   = 5,
			Middle  = 6,
			Output  = 7,
			Footer  = 8,
		};

		Identifier m_id = 0;
		Stage m_stage;

		ImVec2 m_node_min;
		ImVec2 m_node_max;

		ImVec2 m_header_min;
		ImVec2 m_header_max;

		ImVec2 m_content_min;
		ImVec2 m_content_max;

		ImVec2 m_footer_min;
		ImVec2 m_footer_max;

		ImU32 m_header_color;
		ImU32 m_footer_color;
		bool m_has_header;
		bool m_has_footer;

		void transition_to_stage(Stage new_stage);

	public:
		BlueprintBuilder();

		void begin(Identifier id);
		void end();

		void begin_header(const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		void end_header();

		void begin_input(Identifier id);
		void begin_input_pin(Identifier id);
		void end_input_pin();
		void end_input();

		void begin_middle();

		void begin_output(Identifier id);
		void begin_output_pin(Identifier id);
		void end_output_pin();
		void end_output();

		void begin_footer(const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		static void icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color = ImVec4(1, 1, 1, 1),
		                 const ImVec4& inner_color = ImVec4(0, 0, 0, 0));
	};
}// namespace Engine
