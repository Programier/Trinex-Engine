#pragma once

namespace Engine
{
	class Object;
}

namespace Engine::Refl
{
	class Class;
	template<typename T = Engine::Object>
	class SubClassOf final
	{
	private:
		Class* m_class;

	public:
		using Type = T;

		SubClassOf(const SubClassOf&)            = default;
		SubClassOf(SubClassOf&&)                 = default;
		SubClassOf& operator=(const SubClassOf&) = default;
		SubClassOf& operator=(SubClassOf&&)      = default;

		SubClassOf(Class* class_instance = nullptr) : m_class(class_instance) {}

		SubClassOf& operator=(Class* class_instance)
		{
			m_class = class_instance;
			return *this;
		}

		Class* operator->() { return m_class; }

		const Class* operator->() const { return m_class; }

		Class* base() const { return T::static_class_instance(); }

		bool operator==(const SubClassOf& other) const { return m_class == other.m_class; }

		bool operator!=(const SubClassOf& other) const { return !(*this == other); }

		bool operator<(const SubClassOf& other) const { return m_class < other.m_class; }

		bool operator<=(const SubClassOf& other) const { return m_class <= other.m_class; }

		bool operator>(const SubClassOf& other) const { return m_class > other.m_class; }

		bool operator>=(const SubClassOf& other) const { return m_class >= other.m_class; }

		bool operator==(Class* class_instance) const { return m_class == class_instance; }

		bool operator!=(Class* class_instance) const { return m_class != class_instance; }
	};
}// namespace Engine::Refl
