#pragma once
#include <Clients/object_view_client.hpp>

namespace Trinex
{
	class PipelineEditorClient : public ObjectViewClient
	{
		trinex_class(PipelineEditorClient, ObjectViewClient);

	public:
		PipelineEditorClient();
		PipelineEditorClient& select(DeprecatedObject* object) override;
	};
} // namespace Trinex
