#include <Core/logger.hpp>
#include <Graphics/hitbox.hpp>
#include <cstdarg>
#include <cstdio>
#include <editor.hpp>
#include <fstream>
#include <iostream>


class LogWriter : public Engine::Logger
{
public:
    std::ofstream out;
    LogWriter()
    {
        out.open("log.txt");
    }

    Logger& log(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        char buffer[1024];
        vsprintf(buffer, format, args);
        va_end(args);
        std::clog << buffer << std::flush;
        out << buffer << std::flush;
        return *this;
    }

    ~LogWriter()
    {
        out.close();
    }
} Logger;

int main()
try
{
    Engine::logger = &Logger;
    delete &(*new Editor::Application()).loop();
    return 0;
}
catch (const std::exception& e)
{
    Engine::info_log(e.what());
    return 1;
}
