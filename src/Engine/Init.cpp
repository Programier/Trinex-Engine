#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Init/init.hpp>
#include <thread>

#include <iostream>


class EngineInit
{
    bool _M_is_inited = false;

public:
    EngineInit()
    {
        std::clog << "Engine: Start init Engine" << std::endl;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glewExperimental = GL_TRUE;

        // Init glfw
        _M_is_inited = glfwInit() == 1;
        if (_M_is_inited)
            std::clog << "Engine: GLFW init complete" << std::endl;
        else
        {
            std::cerr << "Engine: Failed to init Engine" << std::endl;
            return;
        }


        std::clog << "Engine: Init done" << std::endl;
    }

    ~EngineInit()
    {
        std::clog << "Engine: Terminate" << std::endl;
        glfwTerminate();
        this->_M_is_inited = false;
    }
    bool is_inited()
    {
        return this->_M_is_inited;
    }
} EngineController;


namespace Engine
{
    bool is_inited()
    {
        return EngineController.is_inited();
    }

    bool float_equal(const float& a, const float& b, float e)
    {
        e = e > 0 ? e : -e;
        return a > (b - e) && a < (b + e);
    }
}// namespace Engine
