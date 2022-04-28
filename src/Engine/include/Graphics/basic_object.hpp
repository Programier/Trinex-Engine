#pragma once
#include <BasicFunctional/basic_functional.hpp>
#include <BasicFunctional/reference_wrapper.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <list>


namespace Engine
{
    class ModelMatrixObject
    {
    protected:
        ReferenceWrapper<glm::mat4> _M_models[3] = {ReferenceWrapper<glm::mat4>(), ReferenceWrapper<glm::mat4>(),
                                                    ReferenceWrapper<glm::mat4>()};

        glm::mat4& _M_rotation_matrix();
        glm::mat4& _M_scale_matrix();
        glm::mat4& _M_translate_matrix();


    public:
        const glm::mat4& rotation_matrix() const;
        const glm::mat4& scale_matrix() const;
        const glm::mat4& translate_matrix() const;
        glm::mat4 model() const;
    };


    class TranslateObject : public virtual ModelMatrixObject
    {
    protected:
        ReferenceWrapper<glm::vec3> _M_position = glm::vec3(0.f);

    public:
        TranslateObject();
        TranslateObject& move(const float& x, const float& y, const float& z, const bool& add_values = true);
        TranslateObject& move(const glm::vec3& move_vector, const bool& add_values = true);
        TranslateObject& move(const float& x, const float& y, const float& z, const glm::vec3& right, const glm::vec3& up,
                              const glm::vec3& front, const bool& add_values = true);
        TranslateObject& move(const glm::vec3 move_vector, const glm::vec3& right, const glm::vec3& up, const glm::vec3& front,
                              const bool& add_values = true);
        TranslateObject& move(const float& distance, const glm::vec3& axis, const bool& add_value = true);
        const glm::vec3& position() const;

        TranslateObject& link_to(TranslateObject& obj);
        ~TranslateObject();
    };


    class ScaleObject : public virtual ModelMatrixObject
    {
    protected:
        ReferenceWrapper<glm::vec3> _M_scale = glm::vec3(1.f);

    public:
        ScaleObject();
        const glm::vec3& scale() const;
        ScaleObject& scale(const glm::vec3& sc, const bool& add_values = true);
        ScaleObject& scale(const float& x, const float& y, const float& z, const bool& add_values = true);
        ScaleObject& link_to(ScaleObject& obj);
    };


    class RotateObject : public virtual ModelMatrixObject
    {
    private:
        void update_model(const glm::quat& q, const bool& add_values);

    protected:
        ReferenceWrapper<glm::quat> _M_quaternion = glm::quat(glm::vec3(0.f, 0.f, 0.f));
        ReferenceWrapper<glm::vec3> _M_euler_angles = glm::vec3(0.f, 0.f, 0.f);
        ReferenceWrapper<glm::vec3> _M_front = -OZ;
        ReferenceWrapper<glm::vec3> _M_right = OX;
        ReferenceWrapper<glm::vec3> _M_up = OY;

    public:
        RotateObject();
        const glm::vec3& euler_angles() const;
        RotateObject& rotate(const float& x, const float& y, const float& z, const bool& add_values = true);
        RotateObject& rotate(const glm::vec3& r, const bool& add_values = true);
        RotateObject& rotate(const float& angle, const glm::vec3& axis, const bool& add_values = true);
        RotateObject& rotate(const glm::quat& q, const bool& add_values = true);
        const glm::quat& quaternion() const;
        const glm::vec3& front_vector() const;
        const glm::vec3& right_vector() const;
        const glm::vec3& up_vector() const;

        RotateObject& link_to(RotateObject& obj);
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
    };

}// namespace Engine
