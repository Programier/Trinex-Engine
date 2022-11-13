#include <opengl.hpp>
#include <opengl_object.hpp>

static float _M_line_width = 1.f;


API float api_get_current_line_rendering_width()
{
    return _M_line_width;
}


API void api_set_line_rendering_width(float value)
{
    _M_line_width = value < 0 ? -value : value;
    glLineWidth(_M_line_width);
}
