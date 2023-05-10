#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/logger.hpp>

namespace Engine
{
    int CommandLet::execute(int argc, char** argv)
    {
        logger->log(
                "CommandLet: You must override method 'int execute(int argc, char** argv)' for usin your commandlet!");
        return 0;
    }

    REGISTER_CLASS(Engine::CommandLet, Engine::Object);
}// namespace Engine
