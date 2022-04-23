#pragma once
#include <BasicFunctional/basic_functional.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <list>


namespace Engine
{
    class ModelMatrixObject
    {
    protected:
        glm::mat4 _M_model = glm::mat4(1.0f);

    public:
        const glm::mat4& model() const;
        void link_to(const ModelMatrixObject& object);
    };


    class TranslateObject : public virtual ModelMatrixObject
    {
    private:
        std::list<TranslateObject*> _M_sync_objects = std::list<TranslateObject*>();
        static void private_move(TranslateObject* obj, const glm::vec3 move_vector, const glm::vec3& right, const glm::vec3& up,
                                 const glm::vec3& front, const bool& add_values);

    protected:
        glm::vec3 _M_position = glm::vec3(0.f);

    public:
        TranslateObject& move(const float& x, const float& y, const float& z, const bool& add_values = true);
        TranslateObject& move(const glm::vec3& move_vector, const bool& add_values = true);
        TranslateObject& move(const float& x, const float& y, const float& z, const glm::vec3& right, const glm::vec3& up,
                              const glm::vec3& front, const bool& add_values = true);
        TranslateObject& move(const glm::vec3 move_vector, const glm::vec3& right, const glm::vec3& up, const glm::vec3& front,
                              const bool& add_values = true);
        TranslateObject& move(const float& distance, const glm::vec3& axis, const bool& add_value = true);
        const glm::vec3& position() const;
        TranslateObject& link_to(TranslateObject& object);
        TranslateObject& sync_with(TranslateObject& object, const bool& double_sync = false);
        ~TranslateObject();

        template<typename Object, typename Function, typename... Args>
        friend Object& sync(Object* _this, Function function, const Args&... args);

        template<typename Object>
        friend Object& sync_object_with(Object* base, Object& object, const double& double_sync);
    };


    class ScaleObject : public virtual ModelMatrixObject
    {
    private:
        std::list<ScaleObject*> _M_sync_objects = std::list<ScaleObject*>();

    protected:
        glm::vec3 _M_scale = glm::vec3(1.f);

    public:
        const glm::vec3& scale() const;
        ScaleObject& scale(const glm::vec3& sc, const bool& add_values = true);
        ScaleObject& scale(const float& x, const float& y, const float& z, const bool& add_values = true);
        ScaleObject& link_to(ScaleObject& object);
        ScaleObject& sync_with(ScaleObject& object, const bool& double_sync = false);
        template<typename Object, typename Function, typename... Args>
        friend Object& sync(Object* _this, Function function, const Args&... args);

        template<typename Object>
        friend Object& sync_object_with(Object* base, Object& object, const double& double_sync);
    };


    class RotateObject : public virtual ModelMatrixObject
    {
    private:
        std::list<RotateObject*> _M_sync_objects = std::list<RotateObject*>();
        void update_model(const glm::quat& q, const bool& add_values);

    protected:
        glm::quat _M_quaternion = glm::quat(glm::vec3(0.f, 0.f, 0.f));
        glm::vec3 _M_euler_angles = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 _M_front = -OZ;
        glm::vec3 _M_right = OX;
        glm::vec3 _M_up = OY;

    public:
        const glm::vec3& euler_angles() const;
        RotateObject& rotate(const float& x, const float& y, const float& z, const bool& add_values = true);
        RotateObject& rotate(const glm::vec3& r, const bool& add_values = true);
        RotateObject& rotate(const float& angle, const glm::vec3& axis, const bool& add_values = true);
        RotateObject& rotate(const glm::quat& q, const bool& add_values = true);
        const glm::quat& quaternion() const;
        const glm::vec3& front_vector() const;
        const glm::vec3& right_vector() const;
        const glm::vec3& up_vector() const;
        RotateObject& link_to(RotateObject& object);
        RotateObject& sync_with(RotateObject& object, const bool& double_sync = false);

        template<typename Object, typename Function, typename... Args>
        friend Object& sync(Object* _this, Function function, const Args&... args);

        template<typename Object>
        friend Object& sync_object_with(Object* base, Object& object, const double& double_sync);
    };


    template<typename FirstType = void, typename... Args>
    constexpr bool is_basic_object_base_classes(const bool& void_status = false)
    {
        if (std::is_void<FirstType>::value)
            return void_status;
        constexpr bool status = std::is_same<FirstType, ModelMatrixObject>::value || std::is_same<FirstType, RotateObject>::value ||
                                std::is_same<FirstType, TranslateObject>::value || std::is_same<FirstType, ScaleObject>::value;
        return status && is_basic_object_base_classes<Args...>(true);
    }

    template<typename... BaseClasses>
    class BasicObject : public BaseClasses...
    {
    private:
        template<typename... Classes>
        BasicObject& recursive_link_to(BasicObject<Classes...>&)
        {
            return *this;
        }

        template<typename... Classes, typename Current, typename... Args>
        BasicObject& recursive_link_to(BasicObject<Classes...>& object, Current current, Args... args)
        {
            dynamic_cast<Current&>(*this).link_to(dynamic_cast<Current&>(object));
            return recursive_link_to(object, args...);
        }


        template<typename... Classes>
        BasicObject& recursive_sync_with(BasicObject<Classes...>&, const bool& double_sync)
        {
            return *this;
        }

        template<typename... Classes, typename Current, typename... Args>
        BasicObject& recursive_sync_with(BasicObject<Classes...>& object, const bool& double_sync, Current current, Args... args)
        {
            dynamic_cast<Current&>(*this).sync_with(dynamic_cast<Current&>(object), double_sync);
            return recursive_sync_with(object, double_sync, args...);
        }

    public:
        BasicObject()
        {
            static_assert(is_basic_object_base_classes<BaseClasses...>(), "Cannot inherit from this pack of classes");
        }

        template<typename... Classes>
        BasicObject& link_to(BasicObject<Classes...>& basic_object)
        {
            return recursive_link_to(basic_object, Classes()...);
        }

        template<typename... Classes>
        BasicObject& sync_with(BasicObject<Classes...>& basic_object, const bool& double_sync = false)
        {
            return recursive_sync_with(basic_object, double_sync, Classes()...);
        }
    };

}// namespace Engine
