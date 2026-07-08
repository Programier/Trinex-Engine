#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>

namespace Trinex
{
	struct ENGINE_EXPORT ScriptDiagnostic {
		enum class Type : u8
		{
			Info,
			Warning,
			Error,
		};

		Type type        = Type::Info;
		String section   = {};
		i32 row          = 0;
		i32 column       = 0;
		String message   = {};
	};

	struct ENGINE_EXPORT ScriptBuildResult {
		bool success = false;
		Vector<ScriptDiagnostic> diagnostics;

		bool has_errors() const
		{
			for (const auto& diagnostic : diagnostics)
			{
				if (diagnostic.type == ScriptDiagnostic::Type::Error)
					return true;
			}

			return false;
		}
	};
}// namespace Trinex
