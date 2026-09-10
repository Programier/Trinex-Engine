#include <Core/enums.hpp>
#include <Core/etl/templates.hpp>
#include <Core/stream.hpp>
#include <SDL3/SDL_process.h>
#include <SDLPlatform/process.hpp>

namespace Trinex::Platform
{
	static constexpr const char* s_vfs_file_property = "Trinex.VFS.File";

	struct StreamInfo {
		Stream* src;
		SDL_IOStream* dst;
		const char* option;
		const char* pointer;
	};

	static SDL_IOStream* SDL_GetProcessError(SDL_Process* process)
	{
		if (process == nullptr)
			return nullptr;

		auto props = SDL_GetProcessProperties(process);

		if (props == 0)
			return nullptr;

		return static_cast<SDL_IOStream*>(SDL_GetPointerProperty(props, SDL_PROP_PROCESS_STDOUT_POINTER, nullptr));
	}


	static Sint64 SDLCALL file_seek(void* userdata, Sint64 offset, SDL_IOWhence whence)
	{
		Stream* stream = static_cast<Stream*>(userdata);

		if (!stream->seekable())
			return -1;

		IOWhence dir;

		switch (whence)
		{
			case SDL_IO_SEEK_SET: dir = IOWhence::Begin; break;
			case SDL_IO_SEEK_CUR: dir = IOWhence::Current; break;
			case SDL_IO_SEEK_END: dir = IOWhence::End; break;

			default: return -1;
		}

		return static_cast<Sint64>(stream->offset(offset, dir));
	}

	static size_t SDLCALL file_read(void* userdata, void* ptr, size_t size, SDL_IOStatus* status)
	{
		Stream* file = static_cast<Stream*>(userdata);

		const usize read = file->read(ptr, size);

		if (read == 0)
			*status = SDL_IO_STATUS_EOF;

		return read;
	}

	static size_t SDLCALL file_write(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status)
	{
		Stream* file = static_cast<Stream*>(userdata);

		const usize written = file->write(ptr, size);

		if (written != size)
			*status = SDL_IO_STATUS_WRITEONLY;

		return written;
	}

	static bool SDLCALL file_flush(void* userdata, SDL_IOStatus*)
	{
		return true;
	}

	static bool SDLCALL file_close(void* userdata)
	{
		return true;
	}

	static SDL_IOStream* convert_stream(Stream* file)
	{
		if (!file)
			return nullptr;

		static const SDL_IOStreamInterface interface = []() {
			SDL_IOStreamInterface iface{};
			SDL_INIT_INTERFACE(&iface);

			iface.seek  = file_seek;
			iface.read  = file_read;
			iface.write = file_write;
			iface.flush = file_flush;
			iface.close = file_close;

			return iface;
		}();

		SDL_IOStream* stream = SDL_OpenIO(&interface, file);

		if (stream)
		{
			SDL_SetPointerProperty(SDL_GetIOProperties(stream), s_vfs_file_property, file);
		}

		return stream;
	}

	static Stream* convert_stream(SDL_IOStream* stream)
	{
		if (!stream)
			return nullptr;

		auto props = SDL_GetIOProperties(stream);

		Stream* file = static_cast<Stream*>(SDL_GetPointerProperty(props, s_vfs_file_property, nullptr));

		if (file == nullptr)
		{
		}

		return file;
	}

	static void destroy_stream(SDL_IOStream* stream, Stream* file = nullptr)
	{
		if (stream == nullptr || file == nullptr)
			return;

		auto props = SDL_GetIOProperties(stream);

		if (file == SDL_GetPointerProperty(props, s_vfs_file_property, nullptr))
		{
			SDL_CloseIO(stream);
			return;
		}

		trx_delete file;
	}

	SDLProcess::SDLProcess(SDL_Process* process) : m_process(process)
	{
		m_stdin  = convert_stream(SDL_GetProcessInput(m_process));
		m_stdout = convert_stream(SDL_GetProcessOutput(m_process));
		m_stderr = convert_stream(SDL_GetProcessError(m_process));
	}

	SDLProcess::~SDLProcess()
	{
		destroy_stream(SDL_GetProcessInput(m_process), m_stdin);
		destroy_stream(SDL_GetProcessOutput(m_process), m_stdout);
		destroy_stream(SDL_GetProcessError(m_process), m_stderr);

		SDL_DestroyProcess(m_process);
	}

	bool SDLProcess::kill(bool force)
	{
		return SDL_KillProcess(m_process, force);
	}

	bool SDLProcess::wait(bool block, i32* exitcode)
	{
		return SDL_WaitProcess(m_process, block, exitcode);
	}

	i32 SDLProcess::read(const FunctionRef<void(const u8* data, usize size, i32 code)>& func)
	{
		size_t size;
		int code = -1;

		if (void* data = SDL_ReadProcess(m_process, &size, &code))
		{
			func(static_cast<u8*>(data), size, code);
		}

		return code;
	}

	Stream* SDLProcess::stdin() const
	{
		return nullptr;
	}

	Stream* SDLProcess::stdout() const
	{
		return nullptr;
	}

	Stream* SDLProcess::stderr() const
	{
		return nullptr;
	}

	u64 SDLProcess::pid() const
	{
		return SDL_GetNumberProperty(SDL_GetProcessProperties(m_process), SDL_PROP_PROCESS_PID_NUMBER, 0);
	}

	bool SDLProcess::is_detached() const
	{
		return SDL_GetBooleanProperty(SDL_GetProcessProperties(m_process), SDL_PROP_PROCESS_BACKGROUND_BOOLEAN, false);
	}

	SDLProcessSystem::SDLProcessSystem() {}

	SDLProcessSystem::~SDLProcessSystem() {}

	SDLProcessSystem* SDLProcessSystem::instance()
	{
		static SDLProcessSystem system;
		return &system;
	}

	Process* SDLProcessSystem::launch(const char* const* args, const ProcessOptions* options)
	{
		if (!args || !args[0])
			return nullptr;

		if (!options)
			options = &default_value_of<ProcessOptions>();

		SDL_PropertiesID props = SDL_CreateProperties();

		if (!props)
			return nullptr;

		SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, const_cast<char**>(args));

		if (options->environment)
		{
			SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ENVIRONMENT_POINTER, const_cast<char**>(options->environment));
		}

		if (options->working_directory)
		{
			SDL_SetStringProperty(props, SDL_PROP_PROCESS_CREATE_WORKING_DIRECTORY_STRING, options->working_directory);
		}

		StreamInfo streams[] = {
		        {options->stdin_stream, nullptr, SDL_PROP_PROCESS_CREATE_STDIN_NUMBER, SDL_PROP_PROCESS_CREATE_STDIN_POINTER},
		        {options->stdout_stream, nullptr, SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER, SDL_PROP_PROCESS_CREATE_STDOUT_POINTER},
		        {options->stderr_stream, nullptr, SDL_PROP_PROCESS_CREATE_STDERR_NUMBER, SDL_PROP_PROCESS_CREATE_STDERR_POINTER},
		};


		auto cleanup = [&streams, props]() {
			for (StreamInfo& stream : streams)
			{
				destroy_stream(stream.dst, stream.src);
			}

			SDL_DestroyProperties(props);
		};

		auto setup_stream = [&](StreamInfo& stream, ProcessIO mode) -> bool {
			switch (mode)
			{
				case ProcessIO::Undefined:
				{
					SDL_SetNumberProperty(props, stream.option, SDL_PROCESS_STDIO_NULL);
					return true;
				}

				case ProcessIO::Inherited:
				{
					SDL_SetNumberProperty(props, stream.option, SDL_PROCESS_STDIO_INHERITED);
					return true;
				}

				case ProcessIO::Application:
				{
					SDL_SetNumberProperty(props, stream.option, SDL_PROCESS_STDIO_APP);
					return true;
				}

				case ProcessIO::Redirect:
				{
					if (!stream.src)
					{
						cleanup();
						return false;
					}

					stream.dst = convert_stream(stream.src);

					if (!stream.dst)
					{
						cleanup();
						return false;
					}

					SDL_SetNumberProperty(props, stream.option, SDL_PROCESS_STDIO_REDIRECT);
					SDL_SetPointerProperty(props, stream.pointer, stream.dst);

					return true;
				}
			};

			cleanup();
			return false;
		};

		if (!setup_stream(streams[0], options->stdin_mode))
			return nullptr;

		if (!setup_stream(streams[1], options->stdout_mode))
			return nullptr;

		if (!setup_stream(streams[2], options->stderr_mode))
			return nullptr;

		if (options->detached)
		{
			SDL_SetBooleanProperty(props, SDL_PROP_PROCESS_CREATE_BACKGROUND_BOOLEAN, true);
		}

		SDL_Process* process = SDL_CreateProcessWithProperties(props);

		if (!process)
		{
			cleanup();
			return nullptr;
		}

		SDL_DestroyProperties(props);
		return trx_new SDLProcess(process);
	}

	SDLProcessSystem& SDLProcessSystem::destroy(Process* process)
	{
		if (process)
		{
			trx_delete process;
		}
		return *this;
	}
}// namespace Trinex::Platform
