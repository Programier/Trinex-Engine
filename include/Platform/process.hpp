#pragma once
#include <Core/etl/span.hpp>
#include <Core/etl/string.hpp>
#include <Core/types/path.hpp>
#include <Platform/enums.hpp>
#include <Platform/object.hpp>
#include <Platform/types.hpp>

namespace Trinex::Platform
{
	struct ProcessDesc {
		Path exec;
		Span<String> args;
		Span<EnvironmentVariable> env;
		Path directory;
		ProcessFlags flags = ProcessFlags::InheritEnvironment;
	};

	class ENGINE_EXPORT Process : public Object
	{
	public:
		virtual bool is_running() const           = 0;
		virtual bool result(i32& code) const      = 0;
		virtual bool wait(u64 timeout_ns = ~0ULL) = 0;
		virtual bool terminate(i32 exit_code = 1) = 0;
		virtual Process* parent()                 = 0;
	};

	class ENGINE_EXPORT ProcessSystem
	{
	public:
		static ProcessSystem* instance();

		virtual ~ProcessSystem()                         = default;
		virtual Process* launch(const ProcessDesc& desc) = 0;
		virtual Process* current()                       = 0;
	};
}// namespace Trinex::Platform
