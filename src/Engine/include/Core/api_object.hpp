#pragma once
#include <Core/engine_types.hpp>
#include <Core/object.hpp>

namespace Engine
{
    struct RHI_Object;
    struct RHI_BindingObject;
    struct RHI_Sampler;
    struct RHI_FrameBuffer;
    struct RHI_Texture;
    struct RHI_Shader;
    struct RHI_Pipeline;

    class ENGINE_EXPORT ApiObjectNoBase
    {
    protected:
        Identifier _M_ID = 0;

        union
        {
            RHI_Object* _M_rhi_object;
            RHI_BindingObject* _M_rhi_binding_object;
            RHI_Sampler* _M_rhi_sampler;
            RHI_FrameBuffer* _M_rhi_framebuffer;
            RHI_Texture* _M_rhi_texture;
            RHI_Shader* _M_rhi_shader;
            RHI_Pipeline* _M_rhi_pipeline;
        };


    public:
        ApiObjectNoBase();
        delete_copy_constructors(ApiObjectNoBase);

        Identifier id() const;
        bool has_object() const;
        bool operator==(const ApiObjectNoBase& obj) const;
        bool operator!=(const ApiObjectNoBase& obj) const;
        bool operator<(const ApiObjectNoBase& obj) const;
        bool operator<=(const ApiObjectNoBase& obj) const;
        bool operator>(const ApiObjectNoBase& obj) const;
        bool operator>=(const ApiObjectNoBase& obj) const;


        template<typename T>
        T* get_rhi_object()
        {
            return reinterpret_cast<T*>(_M_rhi_object);
        }

        operator Identifier() const;
        ApiObjectNoBase& destroy();

    protected:
        virtual ~ApiObjectNoBase();
    };

    class ENGINE_EXPORT ApiObject : public Object, public ApiObjectNoBase
    {
    public:
        declare_class(ApiObject, Object);

    public:
        virtual ApiObject& rhi_create();
    };


    class ApiBindingObject : public ApiObject
    {
    public:
        const ApiBindingObject& bind(BindingIndex binding, BindingIndex set = 0) const;
    };


}// namespace Engine
