#pragma once
#include <BasicFunctional/basic_functional.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace Engine
{
    class BasicObjectCallBack
    {
    public:
        virtual void call() = 0;
    };


    class ModelMatrixObject
    {

    protected:
        glm::mat4 _M_model = glm::mat4(1.0f);

    public:
        const glm::mat4& model() const;
    };


    class TranslateObject : public virtual ModelMatrixObject
    {
    protected:
        glm::vec3 _M_position = glm::vec3(0.f);

    public:
        TranslateObject& move(const float& x, const float& y, const float& z, const bool& add_values = true);
        TranslateObject& move(const glm::vec3& move_vector, const bool& add_values = true);
        TranslateObject& move(const float& x, const float& y, const float& z, const glm::vec3& right,
                              const glm::vec3& up, const glm::vec3& front, const bool& add_values = true);
        TranslateObject& move(const glm::vec3 move_vector, const glm::vec3& right, const glm::vec3& up,
                              const glm::vec3& front, const bool& add_values = true);
        TranslateObject& move(const float& distance, const glm::vec3& axis, const bool& add_value = true);
        const glm::vec3& position() const;
    };


    class ScaleObject : public virtual ModelMatrixObject
    {
    protected:
        glm::vec3 _M_scale = glm::vec3(1.f);

    public:
        const glm::vec3& scale() const;
        ScaleObject& scale(const glm::vec3& sc, const bool& add_values = true);
        ScaleObject& scale(const float& x, const float& y, const float& z, const bool& add_values = true);
    };


    class RotateObject : public virtual ModelMatrixObject
    {
    private:
        void update_model(const glm::quat& q, const bool& add_values);

    protected:
        glm::quat _M_quaternion = glm::quat(glm::vec3(0.f, 0.f, 0.f));
        glm::vec3 _M_euler_angles;
        glm::vec3 _M_front = -OZ;
        glm::vec3 _M_right = OX;
        glm::vec3 _M_up = OY;

    public:
        const glm::vec3& euler_angles() const;
        RotateObject& rotate(const float& x, const float& y, const float& z, const bool& add_values = true);
        RotateObject& rotate(const glm::vec3& r, const bool& add_values = true);
        RotateObject& rotate(const float& angle, const glm::vec3& axis, const bool& add_values = true);
        const glm::quat& quaternion() const;
        const glm::vec3& front_vector() const;
        const glm::vec3& right_vector() const;
        const glm::vec3& up_vector() const;
    };

    template<typename... BaseClasses>
    class BasicObject : public BaseClasses...
    {
    };

}// namespace Engine
