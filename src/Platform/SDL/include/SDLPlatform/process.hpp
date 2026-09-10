#pragma once
#include <Platform/process.hpp>

struct SDL_Process;

namespace Trinex::Platform
{
	class ENGINE_EXPORT SDLProcess : public Process
	{
	private:
		SDL_Process* m_process = nullptr;
		Stream* m_stdin        = nullptr;
		Stream* m_stdout       = nullptr;
		Stream* m_stderr       = nullptr;

	public:
		SDLProcess(SDL_Process* process);
		~SDLProcess();

		bool kill(bool force) override;
		bool wait(bool block, i32* exitcode) override;
		i32 read(const FunctionRef<void(const u8* data, usize size, i32 code)>& func) override;

		Stream* stdin() const override;
		Stream* stdout() const override;
		Stream* stderr() const override;

		u64 pid() const override;
		bool is_detached() const override;
	};

	class ENGINE_EXPORT SDLProcessSystem : public ProcessSystem
	{
	private:
		SDLProcessSystem();
		~SDLProcessSystem();

	public:
		static SDLProcessSystem* instance();
		Process* launch(const char* const* args, const ProcessOptions* options = nullptr) override;
		SDLProcessSystem& destroy(Process* process) override;
	};
}// namespace Trinex::Platform
