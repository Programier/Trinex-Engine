#include <opengl_api.hpp>
#include <opengl_types.hpp>

namespace Engine
{
    struct OpenGL_Texture : OpenGL_Object {
        TextureParams _M_params;
        GLuint _M_GL_type;
        GLuint _M_GL_format;
        GLuint _M_GL_pixel_type;

        int_t _M_base_level = 0;
        DepthStencilMode _M_depth_stencil_mode = DepthStencilMode::DEPTH;
        CompareFunc _M_compare_func;
        CompareMode _M_compare_mode;
        int_t _M_max_lod_level = 1000;
        int_t _M_min_lod_level = -1000;
        int_t _M_max_mipmap_level = 1000;
        SwizzleRGBA _M_swizzle = {SwizzleRGBA::SwizzleValue::RED, SwizzleRGBA::SwizzleValue::GREEN,
                                  SwizzleRGBA::SwizzleValue::BLUE, SwizzleRGBA::SwizzleValue::ALPHA};
        WrapValue _M_wrap_s = WrapValue::REPEAT;
        WrapValue _M_wrap_t = WrapValue::REPEAT;
        WrapValue _M_wrap_r = WrapValue::REPEAT;

        void* instance_address() override
        {
            return reinterpret_cast<void*>(this);
        }


        OpenGL_Texture(const TextureParams& params)
        {
            OpenGL::_M_api->_M_current_logger->log("OpenGL: Creating new texture\n");
            this->_M_params = params;

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            // Copy memory from data to data
            this->_M_GL_format = _M_pixel_formats.at(params.format);
            this->_M_GL_type = _M_types.at(params.type);
            this->_M_GL_pixel_type = _M_buffer_value_types.at(params.pixel_type);

            // Creating new texture
            glGenTextures(1, &_M_instance_id);
            glBindTexture(_M_GL_type, _M_instance_id);

            filter(TextureFilter::NEAREST, GL_TEXTURE_MIN_FILTER).filter(TextureFilter::NEAREST, GL_TEXTURE_MAG_FILTER);

            GLint value = 0;
            glGetTexParameteriv(_M_GL_type, GL_TEXTURE_COMPARE_FUNC, &value);
            _M_compare_func = _M_revert_compare_funcs.at(value);
            glGetTexParameteriv(_M_GL_type, GL_TEXTURE_COMPARE_MODE, &value);
            if (value == GL_COMPARE_REF_TO_TEXTURE)
                _M_compare_mode = CompareMode::REF_TO_TEXTURE;
            else
                _M_compare_mode = CompareMode::NONE;
        }

        ~OpenGL_Texture()
        {
            if (_M_instance_id)
            {
                OpenGL::_M_api->_M_current_logger->log("OpenGL: Destroy texture %p\n", this);
                glDeleteTextures(1, &_M_instance_id);
                _M_instance_id = 0;
            }
        }

        OpenGL_Texture& filter(TextureFilter filter, GLuint filter_name)
        {
            glTexParameteri(bind()._M_GL_type, filter_name, _M_texture_filters.at(filter));
            unbind();
            return *this;
        }

        OpenGL_Texture& bind(TextureBindIndex index = 0)
        {
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(_M_GL_type, _M_instance_id);
            return *this;
        }

        OpenGL_Texture& unbind()
        {
            glBindTexture(_M_GL_type, _M_instance_id);
            return *this;
        }

        OpenGL_Texture& generate_mipmap()
        {
            bind();
            glTexParameteri(_M_GL_type, GL_TEXTURE_MAX_LEVEL, _M_max_mipmap_level);
            glTexParameteri(_M_GL_type, GL_TEXTURE_MIN_LOD, _M_min_lod_level);
            glTexParameteri(_M_GL_type, GL_TEXTURE_MAX_LOD, _M_max_lod_level);
            glGenerateMipmap(_M_GL_type);
            unbind();
            return *this;
        }

        OpenGL_Texture& base_level(int_t level)
        {
            bind();
            glTexParameteri(_M_GL_type, GL_TEXTURE_BASE_LEVEL, level);
            unbind()._M_base_level = level;
            return *this;
        }

        OpenGL_Texture& depth_scencil_mode(DepthStencilMode mode)
        {
            bind();
            _M_depth_stencil_mode = mode;
            glTexParameteri(_M_GL_type, GL_DEPTH_STENCIL_TEXTURE_MODE,
                            (mode == DepthStencilMode::DEPTH ? GL_DEPTH_COMPONENT : GL_STENCIL_INDEX));
            unbind();
            return *this;
        }


        OpenGL_Texture& compare_func_texture(CompareFunc func)
        {
            _M_compare_func = func;
            bind();
            glTexParameteri(_M_GL_type, GL_TEXTURE_COMPARE_FUNC, _M_compare_funcs.at(func));
            unbind();
            return *this;
        }

        OpenGL_Texture& compare_mode_texture(CompareMode mode)
        {
            bind();
            glTexParameteri(_M_GL_type, GL_TEXTURE_COMPARE_MODE, _M_compare_modes.at(mode));
            _M_compare_mode = mode;
            return *this;
        }

        TextureFilter filter(GLenum filter_name)
        {
            bind();
            GLint filter = 0;
            glGetTexParameteriv(_M_GL_type, filter_name, &filter);
            try
            {
                return _M_reverse_texture_filters.at(filter);
            }
            catch (...)
            {
                return TextureFilter::NEAREST;
            }
        }

        TextureFilter get_min_filter_texture()
        {
            return filter(GL_TEXTURE_MIN_FILTER);
        }

        TextureFilter get_mag_filter_texture()
        {
            return filter(GL_TEXTURE_MAG_FILTER);
        }

        OpenGL_Texture& set_min_filter_texture(TextureFilter value)
        {
            filter(value, GL_TEXTURE_MIN_FILTER);
            return *this;
        }

        OpenGL_Texture& set_mag_filter_texture(TextureFilter value)
        {
            filter(value, GL_TEXTURE_MAG_FILTER);
            return *this;
        }

        OpenGL_Texture& min_lod_level_texture(int value)
        {
            _M_min_lod_level = value;
            return *this;
        }

        OpenGL_Texture& max_lod_level_texture(int value)
        {
            _M_max_lod_level = value;
            return *this;
        }

        OpenGL_Texture& max_mipmap_level_texture(int value)
        {
            _M_max_mipmap_level = value;
            return *this;
        }

        OpenGL_Texture& swizzle_texture(const SwizzleRGBA& swizzle)
        {
            _M_swizzle = swizzle;
            bind();
            GLint values[4] = {_M_swizzle_values.at(swizzle.R), _M_swizzle_values.at(swizzle.G),
                               _M_swizzle_values.at(swizzle.B), _M_swizzle_values.at(swizzle.A)};
            GLint values2[4] = {GL_TEXTURE_SWIZZLE_R, GL_TEXTURE_SWIZZLE_G, GL_TEXTURE_SWIZZLE_B, GL_TEXTURE_SWIZZLE_A};
            for (int i = 0; i < 4; i++) glTexParameteri(_M_GL_type, values2[i], values[i]);
            return unbind();
        }

        OpenGL_Texture& wrap(WrapValue wrap_value, GLint wrap_type)
        {
            _M_wrap_s = wrap_value;
            bind();
            glTexParameteri(_M_GL_type, wrap_type, _M_wrap_values.at(wrap_value));
            unbind();
            return *this;
        }

        OpenGL_Texture& wrap_s_texture(const WrapValue& wrap_value)
        {
            return wrap(wrap_value, GL_TEXTURE_WRAP_S);
        }

        OpenGL_Texture& wrap_t_texture(const WrapValue& wrap_value)
        {
            return wrap(wrap_value, GL_TEXTURE_WRAP_T);
        }

        OpenGL_Texture& wrap_r_texture(const WrapValue& wrap_value)
        {
            return wrap(wrap_value, GL_TEXTURE_WRAP_R);
        }

        OpenGL_Texture& copy_read_buffer_to_texture_2D(const Size2D& size, const Point2D& pos, int_t mipmap)
        {
            bind();
            glCopyTexImage2D(_M_GL_type, mipmap, _M_GL_format, static_cast<GLint>(pos.x), static_cast<GLint>(pos.y),
                             static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y),
                             static_cast<GLint>(_M_params.border));
            return unbind();
        }

        OpenGL_Texture& update_from_current_read_buffer(const Size2D& size, const Offset2D& offset, const Point2D& pos,
                                                        int_t mipmap)
        {
            bind();
            glCopyTexSubImage2D(_M_GL_type, mipmap, static_cast<GLint>(offset.x), static_cast<GLint>(offset.y),
                                static_cast<GLint>(pos.x), static_cast<GLint>(pos.y), static_cast<GLsizei>(size.x),
                                static_cast<GLsizei>(size.y));
            return unbind();
        }

        GLint get_internal_type_of_texture()
        {
            switch (_M_GL_format)
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
                    return _M_GL_format;
            }
        }

        OpenGL_Texture& gen_texture_2D(const Size2D& size, int_t level, void* data)
        {
            bind();
            glTexImage2D(_M_GL_type, static_cast<GLint>(level), _M_GL_format, static_cast<GLsizei>(size.x),
                         static_cast<GLsizei>(size.y), _M_params.border, get_internal_type_of_texture(),
                         _M_GL_pixel_type, data);
            return unbind();
        }

        OpenGL_Texture& update_texture_2D(const Size2D& size, const Offset2D& offset, int_t level, void* data)
        {
            bind();
            glTexSubImage2D(_M_GL_type, static_cast<GLint>(level), static_cast<GLsizei>(offset.x),
                            static_cast<GLsizei>(offset.y), static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y),
                            _M_GL_format, _M_GL_pixel_type, data);
            return unbind();
        }

        OpenGL_Texture& texture_size(Size3D& size, int_t level)
        {
            bind();
            GLint _size[3];
            glGetTexLevelParameteriv(_M_GL_type, level, GL_TEXTURE_WIDTH, &_size[0]);
            glGetTexLevelParameteriv(_M_GL_type, level, GL_TEXTURE_HEIGHT, &_size[1]);
            glGetTexLevelParameteriv(_M_GL_type, level, GL_TEXTURE_DEPTH, &_size[2]);
            unbind();
            for (int i = 0; i < 3; i++) size[i] = static_cast<decltype(size.x)>(_size[i]);
            return *this;
        }

        OpenGL_Texture& read_texture_2D_data(std::vector<byte>& data, int_t num)
        {
            // Generating framebuffer
            Size3D size;
            texture_size(size, num);
            int_t current_framebuffer = OpenGL::_M_api->get_current_binding(GL_FRAMEBUFFER);


            std::size_t buffer_len = size.x * size.y * size.z * _M_buffer_value_type_sizes.at(_M_params.pixel_type) * 4;
            if (data.size() < buffer_len)
                data.resize(buffer_len);

            GLuint fbo;
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _M_GL_type, _M_instance_id, num);

            glReadPixels(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), _M_GL_format,
                         _M_GL_pixel_type, (void*) data.data());

            glBindFramebuffer(GL_FRAMEBUFFER, current_framebuffer);
            glDeleteFramebuffers(1, &fbo);
            return *this;
        }


        OpenGL_Texture& cubemap_texture_attach_2d_texture(OpenGL_Texture* attach, TextureCubeMapFace index, int_t level)
        {
            bind();
            Size3D size;
            attach->texture_size(size, attach->_M_base_level);
            std::vector<byte> data;
            read_texture_2D_data(data, attach->_M_base_level);

            glTexImage2D(_M_cubemap_indexes.at(index), level, attach->_M_GL_format, size.x, size.y,
                         attach->_M_params.border, attach->_M_GL_format, attach->_M_GL_pixel_type, (void*) data.data());
            return *this;
        }

        OpenGL_Texture& cubemap_texture_attach_data(TextureCubeMapFace index, const Size2D& size, int_t level,
                                                    void* data)
        {
            bind();
            glTexImage2D(_M_cubemap_indexes.at(index), level, _M_GL_format, static_cast<GLsizei>(size.x),
                         static_cast<GLsizei>(size.y), static_cast<GLint>(_M_params.border), _M_GL_format,
                         _M_GL_pixel_type, data);
            return unbind();
        }
    };

    /////////////////// API IMPLEMENTATION //////////////////////

    OpenGL& OpenGL::create_texture(ObjID& ID, const TextureParams& p)
    {
        if (ID)
            destroy_object(ID);
        ID = get_object_id(new OpenGL_Texture(p));
        return *this;
    }

    OpenGL& OpenGL::bind_texture(const ObjID& ID, TextureBindIndex index)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->bind(index);

        return *this;
    }

    const TextureParams* OpenGL::param_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return &obj->get_instance_by_type<OpenGL_Texture>()->_M_params;
        return nullptr;
    }

    int_t OpenGL::base_level_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_base_level;
        return 0;
    }

    OpenGL& OpenGL::base_level_texture(const ObjID& ID, int_t level)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->base_level(level);
        return *this;
    }

    OpenGL& OpenGL::depth_stencil_mode_texture(const ObjID& ID, DepthStencilMode mode)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->depth_scencil_mode(mode);
        return *this;
    }

    DepthStencilMode OpenGL::depth_stencil_mode_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_depth_stencil_mode;
        return DepthStencilMode::DEPTH;
    }


    CompareFunc OpenGL::compare_func_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_compare_func;
        return CompareFunc();
    }

    OpenGL& OpenGL::compare_func_texture(const ObjID& ID, CompareFunc func)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->compare_func_texture(func);
        return *this;
    }

    OpenGL& OpenGL::compare_mode_texture(const ObjID& ID, CompareMode mode)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->compare_mode_texture(mode);
        return *this;
    }

    CompareMode OpenGL::compare_mode_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_compare_mode;
        return CompareMode::NONE;
    }

    TextureFilter OpenGL::min_filter_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->filter(GL_TEXTURE_MIN_FILTER);
        return TextureFilter();
    }

    TextureFilter OpenGL::mag_filter_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->filter(GL_TEXTURE_MAG_FILTER);
        return TextureFilter();
    }

    OpenGL& OpenGL::min_filter_texture(const ObjID& ID, TextureFilter filter_value)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->filter(filter_value, GL_TEXTURE_MIN_FILTER);
        return *this;
    }

    OpenGL& OpenGL::mag_filter_texture(const ObjID& ID, TextureFilter filter_value)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->filter(filter_value, GL_TEXTURE_MAG_FILTER);
        return *this;
    }

    OpenGL& OpenGL::min_lod_level_texture(const ObjID& ID, int_t level)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->_M_min_lod_level = level;
        return *this;
    }

    OpenGL& OpenGL::max_lod_level_texture(const ObjID& ID, int_t level)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->_M_max_lod_level = level;
        return *this;
    }

    OpenGL& OpenGL::max_mipmap_level_texture(const ObjID& ID, int_t level)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->_M_max_mipmap_level = level;
        return *this;
    }

    int_t OpenGL::min_lod_level_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_min_lod_level;
        return 0;
    }

    int_t OpenGL::max_lod_level_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_max_lod_level;
        return 0;
    }

    int_t OpenGL::max_mipmap_level_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_max_mipmap_level;
        return 0;
    }

    OpenGL& OpenGL::swizzle_texture(const ObjID& ID, const SwizzleRGBA& swizzle)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->swizzle_texture(swizzle);
        return *this;
    }

    SwizzleRGBA OpenGL::swizzle_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_swizzle;
        return SwizzleRGBA();
    }

    OpenGL& OpenGL::wrap_s_texture(const ObjID& ID, const WrapValue& value)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->wrap_s_texture(value);
        return *this;
    }

    OpenGL& OpenGL::wrap_t_texture(const ObjID& ID, const WrapValue& value)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->wrap_t_texture(value);
        return *this;
    }

    OpenGL& OpenGL::wrap_r_texture(const ObjID& ID, const WrapValue& value)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->wrap_r_texture(value);
        return *this;
    }

    WrapValue OpenGL::wrap_s_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_wrap_s;
        return WrapValue();
    }

    WrapValue OpenGL::wrap_t_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_wrap_t;
        return WrapValue();
    }

    WrapValue OpenGL::wrap_r_texture(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return obj->get_instance_by_type<OpenGL_Texture>()->_M_wrap_r;
        return WrapValue();
    }

    OpenGL& OpenGL::copy_read_buffer_to_texture_2D(const ObjID& ID, const Size2D& size, const Point2D& point,
                                                   int_t value)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->copy_read_buffer_to_texture_2D(size, point, value);
        return *this;
    }

    OpenGL& OpenGL::texture_2D_update_from_current_read_buffer(const ObjID& ID, const Size2D& size,
                                                               const Offset2D& offset, const Point2D& point,
                                                               int_t mipmap)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->update_from_current_read_buffer(size, offset, point, mipmap);
        return *this;
    }

    OpenGL& OpenGL::gen_texture_2D(const ObjID& ID, const Size2D& size, int_t level, void* data)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->gen_texture_2D(size, level, data);
        return *this;
    }

    OpenGL& OpenGL::update_texture_2D(const ObjID& ID, const Size2D& size, const Offset2D& offset, int_t level,
                                      void* data)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->update_texture_2D(size, offset, level, data);
        return *this;
    }


    OpenGL& OpenGL::texture_size(const ObjID& ID, Size3D& size, int_t level)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->texture_size(size, level);
        return *this;
    }

    OpenGL& OpenGL::generate_texture_mipmap(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->generate_mipmap();
        return *this;
    }

    OpenGL& OpenGL::read_texture_2D_data(const ObjID& ID, std::vector<byte>& data, int_t value)
    {
        data.clear();
        auto obj = instance(ID);
        if (obj)
            obj->get_instance_by_type<OpenGL_Texture>()->read_texture_2D_data(data, value);
        return *this;
    }

    ObjID OpenGL::texture_id(const ObjID& ID)
    {
        auto obj = instance(ID);
        if (obj)
            return static_cast<ObjID>(obj->get_instance_by_type<OpenGL_Texture>()->_M_instance_id);
        return 0;
    }

    OpenGL& OpenGL::cubemap_texture_attach_2d_texture(const ObjID& ID, const ObjID& attach, TextureCubeMapFace face,
                                                      int_t value)
    {
        auto obj = instance(ID);
        auto attach_instance = instance(attach);
        if (obj && attach_instance)
            obj->get_instance_by_type<OpenGL_Texture>()->cubemap_texture_attach_2d_texture(
                    attach_instance->get_instance_by_type<OpenGL_Texture>(), face, value);
        return *this;
    }

    OpenGL& OpenGL::cubemap_texture_attach_data(const ObjID& id, TextureCubeMapFace face, const Size2D& size,
                                                int_t level, void* data)
    {
        auto obj = instance(id);
        if (obj && data)
            obj->get_instance_by_type<OpenGL_Texture>()->cubemap_texture_attach_data(face, size, level, data);
        return *this;
    }

    GLuint OpenGL::get_gl_type_of_texture(const ObjID& ID)
    {
        return instance(ID)->get_instance_by_type<OpenGL_Texture>()->_M_GL_type;
    }

}// namespace Engine
