#pragma once
#include <tuple>


namespace Engine
{
    class OpenGL_CommandBase
    {
    private:
        OpenGL_CommandBase* _M_next = nullptr;

    public:
        virtual OpenGL_CommandBase& apply() = 0;

        virtual ~OpenGL_CommandBase()
        {}

        friend class OpenGL_CommandBuffer;
    };

    template<typename Function, typename... Args>
    class OpenGL_Command : public OpenGL_CommandBase
    {
        Function _M_function;
        std::tuple<Args...> _M_args;

    public:
        OpenGL_Command(Function function, Args... args)
        {
            _M_args     = std::make_tuple(args...);
            _M_function = function;
        }

        OpenGL_CommandBase& apply()
        {
            std::apply(_M_function, _M_args);
            return *this;
        }
    };


    class OpenGL_CommandBuffer
    {
    private:
        OpenGL_CommandBase* _M_command = nullptr;
        OpenGL_CommandBase* _M_last    = nullptr;

    public:
        inline OpenGL_CommandBuffer& apply()
        {
            for (OpenGL_CommandBase* current = _M_command; current; current = current->apply()._M_next)
                ;
            return *this;
        }

        inline OpenGL_CommandBuffer& next(OpenGL_CommandBase* command)
        {
            if (_M_command == nullptr)
            {
                _M_command = _M_last = command;
            }
            else
            {
                _M_last = (_M_last->_M_next = command);
            }
            return *this;
        }

        inline std::size_t length()
        {
            std::size_t result          = 0;
            OpenGL_CommandBase* command = _M_command;

            for (; command; command = command->_M_next, result++)
                ;
            return result;
        }

        ~OpenGL_CommandBuffer()
        {
            while (_M_command)
            {
                _M_last = _M_command->_M_next;
                delete _M_command;
                _M_command = _M_last;
            }
        }
    };
}// namespace Engine
