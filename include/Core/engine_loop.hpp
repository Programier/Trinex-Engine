#include <Core/engine_types.hpp>

namespace Engine
{
	class ENGINE_EXPORT EngineLoop
	{
		i32 preinit(i32 argc, const char** argv);

	public:
		EngineLoop();
		virtual ~EngineLoop();
		void init(i32 argc, const char** argv);
		void update();
		void terminate();
	};
}// namespace Engine
