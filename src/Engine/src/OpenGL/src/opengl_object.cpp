#include <SDL.h>
#include <cstdarg>
#include <cstdio>
#include <opengl_object.hpp>


using namespace Engine;


static void empty_destroy(ObjID& ID)
{}

API void api_destroy_texture_instance(ObjID& ID);
API void api_destroy_mesh(ObjID& ID);
API void destroy_framebuffer(ObjID& ID);

void (*destroy[])(Engine::ObjID& ID) = {empty_destroy, api_destroy_texture_instance, api_destroy_mesh, destroy_framebuffer};


API void api_destroy_object_instance(ObjID& ID)
{
    check_id(ID, );
    auto object = object_of(ID);
    if (object->_M_references == 0)
        return;
    --(object->_M_references);

    if (object->_M_references == 0)
    {
        destroy[static_cast<std::size_t>(object->_M_type)](ID);
        delete object;
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


Logger** engine_logger = nullptr;

Logger* get_external_logger()
{
    if (engine_logger && *engine_logger)
        return *engine_logger;
    return &empty_logger;
}


API void api_set_logger(Logger*& logger)
{
    engine_logger = &logger;
}


int get_current_binding(GLint obj)
{
    int value;
    glGetIntegerv(obj, &value);
    return value;
}
