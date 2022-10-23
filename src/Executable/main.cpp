#include <Application.hpp>
#include <Core/logger.hpp>
#include <cstdarg>
#include <fstream>

using namespace Engine;


std::ofstream out("log.txt");

class RenderDoc_logger : public Engine::Logger
{
public:
    RenderDoc_logger& log(const char* format, ...) override
    {
        va_list args;
        va_start(args, format);
        char buffer[1024];
        vsprintf(buffer, format, args);
        va_end(args);
        out << buffer << std::endl;
        return *this;
    }
} tmp_logger;

int main(int argc, char** argv)
{
    //Engine::logger = &tmp_logger;
    return game_main(argc, argv);
}
