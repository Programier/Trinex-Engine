#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>

namespace Trinex
{
	struct trinex_api ScriptDiagnostic
	{
		enum class Type : u8
		{
			Info,
			Warning,
			Error,
		};

		Type type      = Type::Info;
		String section = {};
		i32 row        = 0;
		i32 column     = 0;
		String message = {};
	};

	struct trinex_api ScriptBuildResult
	{
		bool success = false;
		Array<ScriptDiagnostic> diagnostics;

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
} // namespace Trinex
