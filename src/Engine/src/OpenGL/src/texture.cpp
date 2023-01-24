#include <cstdio>
#include <opengl.hpp>
#include <opengl_object.hpp>
#include <opengl_texture.hpp>
#include <stdexcept>
#include <unordered_map>


namespace Engine
{
    void OpenGL_Texture::destroy()
    {
        if (_M_ID)
        {
            glDeleteTextures(1, &_M_ID);
            _M_ID = 0;
            DEALLOC_INFO;
        }
    }

    declare_cpp_destructor(OpenGL_Texture);

    API void api_generate_texture_mipmap(const ObjID& ID)
    {
        make_texture(texture, ID);
        check(texture, );

        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        glTexParameteri(texture->_M_GL_type, GL_TEXTURE_MAX_LEVEL, texture->_M_max_mipmap_level);
        glTexParameteri(texture->_M_GL_type, GL_TEXTURE_MIN_LOD, texture->_M_min_lod_level);
        glTexParameteri(texture->_M_GL_type, GL_TEXTURE_MAX_LOD, texture->_M_max_lod_level);
        glGenerateMipmap(texture->_M_GL_type);
        glBindTexture(texture->_M_GL_type, 0);
    }


    static void set_filter(const ObjID& ID, TextureFilter filter, GLuint filter_name)
    {
        make_texture(texture, ID);
        check(texture, );
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        glTexParameteri(texture->_M_GL_type, filter_name, _M_texture_filters.at(filter));
        glBindTexture(texture->_M_GL_type, 0);
    }

    API void api_create_texture_instance(ObjID& ID, const TextureParams& params)
    {
        if (ID)
            api_destroy_object_instance(ID);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        OpenGL_Texture* texture = new OpenGL_Texture();

        ID = object_id_of(texture);
        texture->_M_params = params;

        // Copy memory from data to texture->_M_data
        texture->_M_GL_format = _M_pixel_formats.at(params.format);
        texture->_M_GL_type = _M_types.at(params.type);
        texture->_M_GL_pixel_type = _M_buffer_value_types.at(params.pixel_type);

        // Creating new texture
        glGenTextures(1, &texture->_M_ID);
        glBindTexture(texture->_M_GL_type, texture->_M_ID);

        set_filter(ID, TextureFilter::NEAREST, GL_TEXTURE_MIN_FILTER);
        set_filter(ID, TextureFilter::NEAREST, GL_TEXTURE_MAG_FILTER);

        GLint value = 0;
        glGetTexParameteriv(texture->_M_GL_type, GL_TEXTURE_COMPARE_FUNC, &value);
        texture->_M_compare_func = _M_revert_compare_funcs.at(value);
        glGetTexParameteriv(texture->_M_GL_type, GL_TEXTURE_COMPARE_MODE, &value);
        if (value == GL_COMPARE_REF_TO_TEXTURE)
            texture->_M_compare_mode = CompareMode::REF_TO_TEXTURE;
        else
            texture->_M_compare_mode = CompareMode::NONE;
    }

    API void api_bind_texture(const ObjID& ID, unsigned int num)
    {
        make_texture(texture, ID);
        check(texture, );
        glActiveTexture(GL_TEXTURE0 + num);
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
    }

    API const TextureParams* api_param_texture(const ObjID& ID)
    {
        check_id(ID, nullptr);
        return &texture(ID)->_M_params;
    }

    API int api_get_base_level_texture(const ObjID& ID)
    {
        return ID ? texture(ID)->_M_base_level : 0;
    }

    API void api_set_base_level_texture(const ObjID& ID, int level)
    {
        check_id(ID, );

        make_texture(texture, ID);
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        glTexParameteri(texture->_M_GL_type, GL_TEXTURE_BASE_LEVEL, level);
        glBindTexture(texture->_M_GL_type, 0);
        texture->_M_base_level = level;
    }

    API void api_set_depth_stencil_texture(const ObjID& ID, DepthStencilMode mode)
    {
        make_texture(texture, ID);
        check(texture, );
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        texture->_M_depth_stencil_mode = mode;
        glTexParameteri(texture->_M_GL_type, GL_DEPTH_STENCIL_TEXTURE_MODE,
                        (mode == DepthStencilMode::DEPTH ? GL_DEPTH_COMPONENT : GL_STENCIL_INDEX));
        glBindTexture(texture->_M_GL_type, 0);
    }

    API DepthStencilMode api_get_depth_stencil_texture(const ObjID& ID)
    {
        check_id(ID, DepthStencilMode::DEPTH);
        return texture(ID)->_M_depth_stencil_mode;
    }

    API CompareFunc api_get_compare_func_texture(const ObjID& ID)
    {
        if (!ID)
            return CompareFunc();
        return texture(ID)->_M_compare_func;
    }

    API void api_set_compare_func_texture(const ObjID& ID, CompareFunc func)
    {
        make_texture(texture, ID);
        check(texture, );
        texture->_M_compare_func = func;
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        glTexParameteri(texture->_M_GL_type, GL_TEXTURE_COMPARE_FUNC, _M_compare_funcs.at(func));
        glBindTexture(texture->_M_GL_type, 0);
    }

    API void api_set_compare_mode_texture(const ObjID& ID, CompareMode mode)
    {
        make_texture(texture, ID);
        check(texture, );
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        glTexParameteri(texture->_M_GL_type, GL_TEXTURE_COMPARE_MODE, _M_compare_modes.at(mode));
        texture->_M_compare_mode = mode;
    }

    API CompareMode api_get_compare_mode_texture(const ObjID& ID)
    {
        make_texture(texture, ID);
        check(texture, CompareMode());
        return texture->_M_compare_mode;
    }


    static TextureFilter get_filter(const ObjID& ID, GLenum filter_name)
    {
        make_texture(texture, ID);
        check(texture, TextureFilter());

        glBindTexture(texture->_M_GL_type, texture->_M_ID);

        GLint filter = 0;
        glGetTexParameteriv(texture->_M_GL_type, filter_name, &filter);
        try
        {
            return _M_reverse_texture_filters.at(filter);
        }
        catch (...)
        {
            return TextureFilter::NEAREST;
        }
    }

    API TextureFilter api_get_min_filter_texture(const ObjID& ID)
    {
        return get_filter(ID, GL_TEXTURE_MIN_FILTER);
    }

    API TextureFilter api_get_mag_filter_texture(const ObjID& ID)
    {
        return get_filter(ID, GL_TEXTURE_MAG_FILTER);
    }

    API void api_set_min_filter_texture(const ObjID& ID, TextureFilter filter)
    {
        set_filter(ID, filter, GL_TEXTURE_MIN_FILTER);
    }

    API void api_set_mag_filter_texture(const ObjID& ID, TextureFilter filter)
    {
        set_filter(ID, filter, GL_TEXTURE_MAG_FILTER);
    }

    API void api_set_min_lod_level_texture(const ObjID& ID, int value)
    {
        make_texture(texture, ID);
        check(texture, );
        texture->_M_min_lod_level = value;
    }

    API void api_set_max_lod_level_texture(const ObjID& ID, int value)
    {
        make_texture(texture, ID);
        check(texture, );
        texture->_M_max_lod_level = value;
    }

    API void api_set_max_mipmap_level_texture(const ObjID& ID, int value)
    {
        make_texture(texture, ID);
        check(texture, );
        texture->_M_max_mipmap_level = value;
    }

    API int api_get_min_lod_level_texture(const ObjID& ID)
    {
        make_texture(texture, ID);
        check(texture, 0);
        return texture->_M_min_lod_level;
    }

    API int api_get_max_lod_level_texture(const ObjID& ID)
    {
        make_texture(texture, ID);
        check(texture, 0);
        return texture->_M_max_lod_level;
    }

    API int api_get_max_mipmap_level_texture(const ObjID& ID)
    {
        make_texture(texture, ID);
        check(texture, 0);
        return texture->_M_max_mipmap_level;
    }

    API void api_set_swizzle_texture(const ObjID& ID, const SwizzleRGBA& swizzle)
    {
        make_texture(texture, ID);
        check(texture, );
        texture->_M_swizzle = swizzle;
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        GLint values[4] = {_M_swizzle_values.at(swizzle.R), _M_swizzle_values.at(swizzle.G), _M_swizzle_values.at(swizzle.B),
                           _M_swizzle_values.at(swizzle.A)};
        GLint values2[4] = {GL_TEXTURE_SWIZZLE_R, GL_TEXTURE_SWIZZLE_G, GL_TEXTURE_SWIZZLE_B, GL_TEXTURE_SWIZZLE_A};
        for (int i = 0; i < 4; i++) glTexParameteri(texture->_M_GL_type, values2[i], values[i]);
        glBindTexture(texture->_M_GL_type, 0);
    }

    API SwizzleRGBA api_get_swizzle_texture(const ObjID& ID)
    {
        make_texture(texture, ID);
        check(texture, SwizzleRGBA());
        return texture->_M_swizzle;
    }

    static OpenGL_Texture* set_wrap(const ObjID& ID, WrapValue wrap, GLint wrap_type)
    {
        make_texture(texture, ID);
        check(texture, nullptr);
        texture->_M_wrap_s = wrap;
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        glTexParameteri(texture->_M_GL_type, wrap_type, _M_wrap_values.at(wrap));
        glBindTexture(texture->_M_GL_type, 0);
        return texture;
    }

    API void api_set_wrap_s_texture(const ObjID& ID, const WrapValue& wrap)
    {
        auto texture = set_wrap(ID, wrap, GL_TEXTURE_WRAP_S);
        if (texture)
            texture->_M_wrap_s = wrap;
    }

    API void api_set_wrap_t_texture(const ObjID& ID, const WrapValue& wrap)
    {
        auto texture = set_wrap(ID, wrap, GL_TEXTURE_WRAP_T);
        if (texture)
            texture->_M_wrap_t = wrap;
    }

    API void api_set_wrap_r_texture(const ObjID& ID, const WrapValue& wrap)
    {
        auto texture = set_wrap(ID, wrap, GL_TEXTURE_WRAP_R);
        if (texture)
            texture->_M_wrap_r = wrap;
    }


    API WrapValue api_get_wrap_s_texture(const ObjID& ID)
    {
        make_texture(texture, ID);
        check(texture, WrapValue());
        return texture->_M_wrap_s;
    }

    API WrapValue api_get_wrap_t_texture(const ObjID& ID)
    {
        make_texture(texture, ID);
        check(texture, WrapValue());
        return texture->_M_wrap_t;
    }

    API WrapValue api_get_wrap_r_texture(const ObjID& ID, WrapValue wrap)
    {
        make_texture(texture, ID);
        check(texture, WrapValue());
        return texture->_M_wrap_r;
    }

    API void api_copy_read_buffer_to_texture_2D(const ObjID& ID, const Size2D& size, const Point2D& pos, int mipmap)
    {
        make_texture(texture, ID);
        check(texture, );
        glBindTexture(texture->_M_GL_type, texture->_M_ID);

        glCopyTexImage2D(texture->_M_GL_type, mipmap, texture->_M_GL_format, static_cast<GLint>(pos.x),
                         static_cast<GLint>(pos.y), static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y),
                         static_cast<GLint>(texture->_M_params.border));
        glBindTexture(texture->_M_GL_type, 0);
    }

    API void api_texture_2D_update_from_current_read_buffer(const ObjID& ID, const Size2D& size, const Offset2D& offset,
                                                            const Point2D& pos, int mipmap)
    {
        make_texture(texture, ID);
        check(texture, );
        glBindTexture(texture->_M_GL_type, texture->_M_ID);

        glCopyTexSubImage2D(texture->_M_GL_type, mipmap, static_cast<GLint>(offset.x), static_cast<GLint>(offset.y),
                            static_cast<GLint>(pos.x), static_cast<GLint>(pos.y), static_cast<GLsizei>(size.x),
                            static_cast<GLsizei>(size.y));

        glBindTexture(texture->_M_GL_type, 0);
    }

    static GLint get_internal_type_of_texture(GLint texture)
    {
        switch (texture)
        {
            case GL_STENCIL_INDEX8:
                return GL_STENCIL_INDEX;
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32F:
                return GL_DEPTH_COMPONENT;
            case GL_DEPTH24_STENCIL8:
            case GL_DEPTH32F_STENCIL8:
                return GL_DEPTH_STENCIL;
            default:
                return texture;
        }
    }

    API void api_gen_texture_2D(const ObjID& ID, const Size2D& size, int level, void* data)
    {
        make_texture(texture, ID);
        check(texture, );
        glBindTexture(texture->_M_GL_type, texture->_M_ID);

        glTexImage2D(texture->_M_GL_type, static_cast<GLint>(level), texture->_M_GL_format, static_cast<GLsizei>(size.x),
                     static_cast<GLsizei>(size.y), texture->_M_params.border,
                     get_internal_type_of_texture(texture->_M_GL_format), texture->_M_GL_pixel_type, data);
        glBindTexture(texture->_M_GL_type, 0);
    }

    API void api_update_texture_2D(const ObjID& ID, const Size2D& size, const Offset2D& offset, int level, void* data)
    {
        make_texture(texture, ID);
        check(texture, );
        glBindTexture(texture->_M_GL_type, texture->_M_ID);

        glTexSubImage2D(texture->_M_GL_type, static_cast<GLint>(level), static_cast<GLsizei>(offset.x),
                        static_cast<GLsizei>(offset.y), static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y),
                        texture->_M_GL_format, texture->_M_GL_pixel_type, data);

        glBindTexture(texture->_M_GL_type, 0);
    }

    API void api_get_size_texture(const ObjID& ID, Size3D& size, int level)
    {
        make_texture(texture, ID);
        check(texture, );
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        GLint _size[3];
        glGetTexLevelParameteriv(texture->_M_GL_type, level, GL_TEXTURE_WIDTH, &_size[0]);
        glGetTexLevelParameteriv(texture->_M_GL_type, level, GL_TEXTURE_HEIGHT, &_size[1]);
        glGetTexLevelParameteriv(texture->_M_GL_type, level, GL_TEXTURE_DEPTH, &_size[2]);
        glBindTexture(texture->_M_GL_type, 0);

        for (int i = 0; i < 3; i++) size[i] = static_cast<decltype(size.x)>(_size[i]);
    }

    API void api_read_texture_2D_data(const ObjID& ID, std::vector<byte>& data, int num)
    {
        // Generating framebuffer
        Size3D size;
        api_get_size_texture(ID, size, num);
        int current_framebuffer = get_current_binding(GL_FRAMEBUFFER);
        make_texture(texture, ID);
        check(texture, );

        std::size_t buffer_len = size.x * size.y * size.z * _M_buffer_value_type_sizes.at(texture->_M_params.pixel_type) * 4;
        if (data.size() < buffer_len)
            data.resize(buffer_len);

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->_M_GL_type, texture->_M_ID, num);

        glReadPixels(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), texture->_M_GL_format,
                     texture->_M_GL_pixel_type, (void*) data.data());

        glBindFramebuffer(GL_FRAMEBUFFER, current_framebuffer);
        glDeleteFramebuffers(1, &fbo);
    }


    API ObjID api_texture_id(const ObjID& ID)
    {
        check_id(ID, 0);
        return texture(ID)->_M_ID;
    }

    API void api_cubemap_texture_attach_2d_texture(const ObjID& ID, const ObjID& attach, TextureCubeMapFace index, int level)
    {
        check_id(ID, );
        check_id(attach, );

        make_texture(texture, ID);
        make_texture(attach_texture, attach);
        check(texture, );
        check(attach_texture, );
        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        Size3D size;
        api_get_size_texture(attach, size, attach_texture->_M_base_level);

        std::vector<byte> data;
        api_read_texture_2D_data(attach, data, attach_texture->_M_base_level);

        glTexImage2D(_M_cubemap_indexes.at(index), level, attach_texture->_M_GL_format, size.x, size.y,
                     attach_texture->_M_params.border, attach_texture->_M_GL_format, attach_texture->_M_GL_pixel_type,
                     (void*) data.data());
    }

    API void api_cubemap_texture_attach_data(const ObjID& ID, TextureCubeMapFace index, const Size2D& size, int level,
                                             void* data)
    {
        make_texture(texture, ID);
        check(texture, );

        glBindTexture(texture->_M_GL_type, texture->_M_ID);
        glTexImage2D(_M_cubemap_indexes.at(index), level, texture->_M_GL_format, static_cast<GLsizei>(size.x),
                     static_cast<GLsizei>(size.y), static_cast<GLint>(texture->_M_params.border), texture->_M_GL_format,
                     texture->_M_GL_pixel_type, data);
    }

}// namespace Engine
