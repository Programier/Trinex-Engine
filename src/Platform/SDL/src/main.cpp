#include <Core/engine.hpp>
#include <SDL3/SDL_main.h>

FORCE_ENGINE_EXPORT int main(int argc, char** argv)
{
	return Trinex::Engine::launch(argc, argv);
}
