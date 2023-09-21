#pragma once
#include <Core/engine_types.hpp>
#include <Core/object.hpp>

namespace Engine
{

    namespace RHI
    {
        struct RHI_Object;
        struct RHI_BindingObject;
        struct RHI_Sampler;
        struct RHI_FrameBuffer;
        struct RHI_Texture;
    }// namespace RHI

    class ENGINE_EXPORT ApiObjectNoBase
    {
    protected:
        Identifier _M_ID = 0;

        union
        {
            RHI::RHI_Object* _M_rhi_object;
            RHI::RHI_BindingObject* _M_rhi_binding_object;
            RHI::RHI_Sampler* _M_rhi_sampler;
            RHI::RHI_FrameBuffer* _M_rhi_framebuffer;
            RHI::RHI_Texture* _M_rhi_texture;
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
    };


    class ApiBindingObject : public ApiObject
    {
    public:
        const ApiBindingObject& bind(BindingIndex binding, BindingIndex set = 0) const;
    };


}// namespace Engine
