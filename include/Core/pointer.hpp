#pragma once

#include <Core/etl/hash.hpp>
#include <Core/export.hpp>

namespace Engine
{
	class Object;

	class ENGINE_EXPORT PointerBase
	{
	protected:
		PointerBase& add_reference(Object* object);
		PointerBase& remove_reference(Object* object);
		PointerBase();

		bool serialize(class Archive& ar, Object*& object, bool is_reference);
	};


	template<class InstanceClass>
	class Pointer : private PointerBase
	{
	private:
		union
		{
			InstanceClass* m_instance = nullptr;
			Object* m_object;
		};


	public:
		using Type = InstanceClass;

		struct HashStruct : public Hash<InstanceClass*> {
			size_t operator()(const Pointer<InstanceClass>& instance) const
			{
				return static_cast<Hash<InstanceClass*>>(*this)(instance.m_instance);
			}
		};

		Pointer(InstanceClass* instance = nullptr) : m_instance(instance)
		{
			add_reference(m_object);
		}

		Pointer(const Pointer& pointer)
		{
			*this = pointer;
		}

		Pointer(Pointer&& pointer)
		{
			m_instance         = pointer.m_instance;
			pointer.m_instance = nullptr;
		}

		Pointer& operator=(const Pointer& pointer)
		{
			if (this == &pointer)
				return *this;

			remove_reference(m_object);
			m_instance = pointer.m_instance;
			add_reference(m_object);
			return *this;
		}

		Pointer& operator=(Pointer&& pointer)
		{
			if (this != &pointer)
			{
				remove_reference(m_object);
				m_instance         = pointer.m_instance;
				pointer.m_instance = nullptr;
			}

			return *this;
		}

		Pointer& operator=(InstanceClass* instance)
		{
			remove_reference(m_object);
			m_instance = instance;
			add_reference(m_object);
			return *this;
		}


		InstanceClass* operator->() const
		{
			return m_instance;
		}

		operator InstanceClass*() const
		{
			return m_instance;
		}

		InstanceClass* ptr() const
		{
			return m_instance;
		}

		bool operator==(const Pointer<InstanceClass>& instance) const
		{
			return m_instance == instance.m_instance;
		}

		bool operator!=(const Pointer<InstanceClass>& instance) const
		{
			return m_instance != instance.m_instance;
		}

		bool operator==(const InstanceClass* instance) const
		{
			return m_instance == instance;
		}

		bool operator!=(const InstanceClass* instance) const
		{
			return m_instance != instance;
		}

		~Pointer()
		{
			remove_reference(m_object);
		}


		bool serialize(class Archive& ar, bool is_reference = true)
		{
			return PointerBase::serialize(ar, m_object, is_reference);
		}
	};
}// namespace Engine
