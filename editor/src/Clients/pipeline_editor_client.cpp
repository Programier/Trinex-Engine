#include <Clients/pipeline_editor_client.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader_compiler.hpp>
#include <imgui.h>

namespace Engine
{
	trinex_implement_engine_class(PipelineEditorClient, 0)
	{
		register_client(Pipeline::static_reflection(), This::static_reflection());
	}

	PipelineEditorClient::PipelineEditorClient()
	{
		menu_bar.create("")->actions.push([this]() {
			Object* object = selected_object();

			// if (ImGui::MenuItem("editor/Compile"_localized, nullptr, false, object != nullptr))
			// {
			// 	if (Pipeline* pipeline = instance_cast<Pipeline>(object))
			// 	{
			// 		String source;
			// 		if (!pipeline->shader_source(source))
			// 		{
			// 			error_log("PipelineEditorClient", "Failed to get pipeline source!");
			// 			return;
			// 		}

			// 		ShaderCompiler* compiler = ShaderCompiler::instance();
			// 		if (!compiler)
			// 		{
			// 			error_log("PipelineEditorClient", "Failed to get compiler!");
			// 			return;
			// 		}

			// 		compiler->compile(source, pipeline);
			// 	}
			// 	else
			// 	{
			// 		error_log("PipelineEditorClient", "Failed to cast object to pipeline!");
			// 	}
			// }
		});
	}

	PipelineEditorClient& PipelineEditorClient::select(Object* object)
	{
		if (!object || !object->is_instance_of<Pipeline>())
			return *this;
		Super::select(object);
		return *this;
	}
}// namespace Engine
