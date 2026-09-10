#pragma once
#include <Core/etl/function.hpp>
#include <Platform/enums.hpp>
#include <Platform/types.hpp>

namespace Trinex
{
	class Stream;
}

namespace Trinex::Platform
{
	struct ProcessOptions {
		const char* const* environment = nullptr;
		const char* working_directory  = nullptr;

		ProcessIO stdin_mode  = ProcessIO::Inherited;
		ProcessIO stdout_mode = ProcessIO::Inherited;
		ProcessIO stderr_mode = ProcessIO::Inherited;

		Stream* stdin_stream  = nullptr;
		Stream* stdout_stream = nullptr;
		Stream* stderr_stream = nullptr;

		bool detached = false;
	};

	class ENGINE_EXPORT Process
	{
	public:
		virtual ~Process();

		virtual bool kill(bool force)                                                         = 0;
		virtual bool wait(bool block, i32* exitcode)                                          = 0;
		virtual i32 read(const FunctionRef<void(const u8* data, usize size, i32 code)>& func) = 0;

		virtual Stream* stdin() const    = 0;
		virtual Stream* stdout() const   = 0;
		virtual Stream* stderr() const   = 0;
		virtual u64 pid() const          = 0;
		virtual bool is_detached() const = 0;
	};

	class ENGINE_EXPORT ProcessSystem
	{
	public:
		static ProcessSystem* instance();

		virtual ~ProcessSystem();
		virtual Process* launch(const char* const* args, const ProcessOptions* options = nullptr) = 0;
		virtual ProcessSystem& destroy(Process* process)                                          = 0;
	};
}// namespace Trinex::Platform
