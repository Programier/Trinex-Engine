#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/variant.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/vector.hpp>
#include <Core/name.hpp>

namespace Engine
{
	class RHITexture;
	class RHISampler;
	class RHIShaderResourceView;

	class ENGINE_EXPORT MaterialBindings
	{
	public:
		struct MemoryBlock {
			void* memory = nullptr;
			size_t size  = 0;
		};

		struct CombinedSamplerImage {
			RHIShaderResourceView* texture = nullptr;
			RHISampler* sampler            = nullptr;
		};

		using Binding = Variant<bool, Vector2b, Vector3b, Vector4b,  //
		                        int_t, Vector2i, Vector3i, Vector4i, //
		                        uint_t, Vector2u, Vector3u, Vector4u,//
		                        float, Vector2f, Vector3f, Vector4f, //
		                        MemoryBlock, CombinedSamplerImage,   //
		                        RHISampler*, RHIShaderResourceView*>;

	private:
		using Element = Pair<Name, Binding>;
		Vector<Element> m_bindings;

	private:
		MaterialBindings& sort();

	public:
		MaterialBindings()                                       = default;
		MaterialBindings(const MaterialBindings&)                = default;
		MaterialBindings& operator=(const MaterialBindings&)     = default;
		MaterialBindings(MaterialBindings&&) noexcept            = default;
		MaterialBindings& operator=(MaterialBindings&&) noexcept = default;
		~MaterialBindings()                                      = default;

		MaterialBindings(const std::initializer_list<Pair<Name, Binding>>& list);
		MaterialBindings(const Vector<Pair<Name, Binding>>& list);
		MaterialBindings(Vector<Pair<Name, Binding>>&& list);

		Binding* find_or_create(const Name& name);
		bool unbind(const Name& name);
		const Binding* find(const Name& name) const;


		inline Binding* find(const Name& name)
		{
			return const_cast<Binding*>(const_cast<const MaterialBindings*>(this)->find(name));
		}

		inline Binding* bind(const Name& name, const Binding& binding)
		{
			Binding* dst = find_or_create(name);
			(*dst)       = binding;
			return dst;
		}

		inline Binding* bind(const Name& name, Binding&& binding)
		{
			Binding* dst = find_or_create(name);
			(*dst)       = std::move(binding);
			return dst;
		}

		inline MaterialBindings& clear()
		{
			m_bindings.clear();
			return *this;
		}

		inline size_t size() const { return m_bindings.size(); }
		inline bool empty() const { return m_bindings.empty(); }

		inline Vector<Element>::const_iterator begin() const { return m_bindings.begin(); }
		inline Vector<Element>::const_iterator end() const { return m_bindings.end(); }
		inline Vector<Element>::const_reverse_iterator rbegin() const { return m_bindings.rbegin(); }
		inline Vector<Element>::const_reverse_iterator rend() const { return m_bindings.rend(); }

		inline Vector<Element>::const_iterator cbegin() const { return m_bindings.begin(); }
		inline Vector<Element>::const_iterator cend() const { return m_bindings.end(); }
		inline Vector<Element>::const_reverse_iterator crbegin() const { return m_bindings.crbegin(); }
		inline Vector<Element>::const_reverse_iterator crend() const { return m_bindings.crend(); }
	};
}// namespace Engine
