#include <SDL.h>
#include <cstdarg>
#include <cstdio>
#include <opengl_object.hpp>


using namespace Engine;

OpenGL_Object::OpenGL_Object()
{
    _M_references = 1;
}

void OpenGL_Object::destroy()
{}

OpenGL_Object::~OpenGL_Object()
{}

API void api_destroy_object_instance(ObjID& ID)
{
    auto object = object_of<OpenGL_Object>(ID);
    check(object, );

    if (object)
    {
        if (object->_M_references == 0)
            return;

        --(object->_M_references);

        if (object->_M_references == 0)
        {
            object->destroy();
            delete object;
        }
    }
    ID = 0;
}

API void api_link_object(const ObjID& SRC_ID, ObjID& DST_ID)
{
    if (!SRC_ID)
        return;

    if (DST_ID)
        api_destroy_object_instance(DST_ID);

    DST_ID = SRC_ID;
    ++(object_of(SRC_ID)->_M_references);
}

ObjID object_id_of(const OpenGL_Object* object)
{
    return reinterpret_cast<ObjID>(object);
}


class EmptyLogger : public Engine::Logger
{
public:
    EmptyLogger& log(const char* format, ...) override
    {
        va_list args;
        va_start(args, format);
        char buffer[1024];
        vsprintf(buffer, format, args);
        va_end(args);
        SDL_Log("%s", buffer);
        return *this;
        return *this;
    }

} empty_logger;


Logger* external_logger = nullptr;


API void api_set_logger(Logger*& logger)
{

    external_logger = logger ? logger : &empty_logger;
}


int get_current_binding(GLint obj)
{
    int value;
    glGetIntegerv(obj, &value);
    return value;
}
