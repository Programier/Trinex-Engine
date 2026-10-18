#include <Core/engine.hpp>
#include <SDL3/SDL_main.h>

trinex_export int main(int argc, char** argv)
{
	return Trinex::Engine::launch(argc, argv);
}
