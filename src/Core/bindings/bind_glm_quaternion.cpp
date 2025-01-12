#include "bind_glm.hpp"
#include <Core/engine_loading_controllers.hpp>

namespace Engine
{

	struct QuaternionWrapper : public Quaternion {
		Quaternion& obj()
		{
			return *this;
		}

		const Quaternion& obj() const
		{
			return *this;
		}

		QuaternionWrapper()
		{}

		template<typename... Args>
		QuaternionWrapper(Args... args) : Quaternion(args...)
		{}

		QuaternionWrapper(const QuaternionWrapper& vec)
		{
			obj() = vec.obj();
		}

		QuaternionWrapper(Quaternion::value_type v)
		{
			obj().x = v;
			obj().y = v;
			obj().z = v;
			obj().w = v;
		}

		bool operator==(const QuaternionWrapper& wrap) const
		{
			return obj() == wrap.obj();
		}

		QuaternionWrapper& operator=(const QuaternionWrapper& new_obj)
		{
			obj() = new_obj.obj();
			return *this;
		}

		QuaternionWrapper& operator+=(const QuaternionWrapper& new_obj)
		{
			obj() += new_obj.obj();
			return *this;
		}

		QuaternionWrapper& operator-=(const QuaternionWrapper& new_obj)
		{
			obj() -= new_obj.obj();
			return *this;
		}

		QuaternionWrapper& operator*=(const QuaternionWrapper& new_obj)
		{
			obj() *= new_obj.obj();
			return *this;
		}

		QuaternionWrapper& operator/=(Quaternion::value_type v)
		{
			obj() /= v;
			return *this;
		}

		QuaternionWrapper& operator*=(Quaternion::value_type v)
		{
			obj() *= v;
			return *this;
		}

		QuaternionWrapper operator+(const QuaternionWrapper& new_obj) const
		{
			return obj() + new_obj.obj();
		}


		QuaternionWrapper operator-(const QuaternionWrapper& new_obj) const
		{
			return obj() - new_obj.obj();
		}

		QuaternionWrapper operator*(const QuaternionWrapper& new_obj) const
		{
			return obj() * new_obj.obj();
		}

		QuaternionWrapper operator*(Quaternion::value_type v) const
		{
			return obj() * v;
		}

		QuaternionWrapper operator/(Quaternion::value_type v) const
		{
			return obj() / v;
		}
	};

	static void bind_quat_operators(ScriptClassRegistrar& registrar)
	{
		using T = QuaternionWrapper;


		registrar.method("Engine::Quaternion& opAssign(const Engine::Quaternion&)", method_of<T&>(&T::operator=),
		                 ScriptCallConv::ThisCall);

		registrar.method("bool opEquals(const Engine::Quaternion&) const", method_of<bool>(&T::operator==),
		                 ScriptCallConv::ThisCall);

		registrar.method("Engine::Quaternion& opAddAssign(const Engine::Quaternion&)", method_of<T&, const T&>(&T::operator+=),
		                 ScriptCallConv::ThisCall);

		registrar.method("Engine::Quaternion& opSubAssign(const Engine::Quaternion&)", method_of<T&, const T&>(&T::operator-=),
		                 ScriptCallConv::ThisCall);

		registrar.method("Engine::Quaternion& opMulAssign(const Engine::Quaternion&)", method_of<T&, const T&>(&T::operator*=),
		                 ScriptCallConv::ThisCall);
		registrar.method("Engine::Quaternion& opMulAssign(float)", method_of<T&, typename T::value_type>(&T::operator*=),
		                 ScriptCallConv::ThisCall);

		registrar.method("Engine::Quaternion& opDivAssign(const Engine::Quaternion&)",
		                 method_of<T&, typename T::value_type>(&T::operator/=), ScriptCallConv::ThisCall);

		registrar.method("Engine::Quaternion opAdd(const Engine::Quaternion&) const", method_of<T, const T&>(&T::operator+),
		                 ScriptCallConv::ThisCall);

		registrar.method("Engine::Quaternion opSub(const Engine::Quaternion&) const", method_of<T, const T&>(&T::operator-),
		                 ScriptCallConv::ThisCall);

		registrar.method("Engine::Quaternion opMul(const Engine::Quaternion&) const", method_of<T, const T&>(&T::operator*),
		                 ScriptCallConv::ThisCall);
		registrar.method("Engine::Quaternion opMul(float) const", method_of<T, typename T::value_type>(&T::operator*),
		                 ScriptCallConv::ThisCall);
		registrar.method("Engine::Quaternion opMul_r(float) const", method_of<T, typename T::value_type>(&T::operator*),
		                 ScriptCallConv::ThisCall);

		registrar.method("Engine::Quaternion opDiv(float) const", method_of<T, typename T::value_type>(&T::operator/),
		                 ScriptCallConv::ThisCall);
	}


	static void bind_quat_props(ScriptClassRegistrar& registrar)
	{
		using T = QuaternionWrapper;
		registrar.property("float w", &T::w);
		registrar.property("float x", &T::x);
		registrar.property("float y", &T::y);
		registrar.property("float z", &T::z);
	}


	static void on_init()
	{
		using RegistryType1 = QuaternionWrapper;

		{
			ScriptClassRegistrar registrar =
			        ScriptClassRegistrar::value_class("Engine::Quaternion", sizeof(RegistryType1), info_of<RegistryType1>());
			bind_glm_behaviours<QuaternionWrapper>(registrar, "float");
			bind_quat_operators(registrar);
			bind_quat_props(registrar);
		}
	}

	static ReflectionInitializeController controller(on_init, "Engine::Quaternion");
}// namespace Engine
