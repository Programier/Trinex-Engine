#include <Core/archive.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/array.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/map.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/sampler.hpp>
#include <RHI/initializers.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	trinex_implement_struct(Engine::Sampler, 0)
	{
		trinex_refl_virtual_prop(filter, filter, filter, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(address_u, address_u, address_u, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(address_v, address_v, address_v, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(address_w, address_w, address_w, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(compare_func, compare_func, compare_func, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(border_color, border_color, border_color, Refl::Property::IsTransient);
	}

	struct SamplerInitializerHash {
		inline size_t operator()(const RHISamplerInitializer& initializer) const { return initializer.hash(); }
	};

	using SamplersMap = Map<RHISamplerInitializer, class SamplerImpl*, SamplerInitializerHash>;

	static SamplersMap s_samplers;
	static Array<SamplerImpl*, 3> s_default_samplers;
	static SamplerImpl* s_first_sampler = nullptr;
	static SamplerImpl* s_last_sampler  = nullptr;
	static bool s_gc_disabled           = false;

	class SamplerImpl
	{
	private:
		Atomic<uint64_t> m_references = 0;
		RHISampler* m_sampler         = nullptr;
		SamplerImpl* m_prev           = nullptr;
		SamplerImpl* m_next           = nullptr;
		RHISamplerInitializer m_initializer;

	public:
		SamplerImpl(const RHISamplerInitializer& initializer) : m_initializer(initializer)
		{
			if (s_last_sampler)
			{
				s_last_sampler->m_next = this;
				m_prev                 = s_last_sampler;
				s_last_sampler         = this;
			}
			else
			{
				s_first_sampler = s_last_sampler = this;
			}
		}

		SamplerImpl& initialize()
		{
			is_in_logic_thread_checked();
			render_thread()->call([this]() { m_sampler = rhi->create_sampler(&m_initializer); });
			s_samplers[m_initializer] = this;
			return *this;
		}

		inline void add_ref() { ++m_references; }

		inline void release()
		{
			--m_references;

			if (s_gc_disabled && m_references == 0)
			{
				trx_delete this;
			}
		}

		inline uint64_t references() const { return m_references; }
		inline const RHISamplerInitializer& initializer() const { return m_initializer; }
		inline RHISampler* rhi_sampler() const { return m_sampler; }
		inline SamplerImpl* prev() const { return m_prev; }
		inline SamplerImpl* next() const { return m_next; }

		static inline SamplerImpl* static_find_or_create(const RHISamplerInitializer& initializer)
		{
			auto it = s_samplers.find(initializer);

			if (it == s_samplers.end())
				return &((trx_new SamplerImpl(initializer))->initialize());

			return it->second;
		}

		~SamplerImpl()
		{
			if (m_sampler)
			{
				render_thread()->call([sampler = m_sampler]() { sampler->release(); });
			}

			s_samplers.erase(m_initializer);

			if (m_prev)
			{
				m_prev->m_next = m_next;
			}
			else
			{
				s_first_sampler = m_next;
			}

			if (m_next)
			{
				m_next->m_prev = m_prev;
			}
			else
			{
				s_last_sampler = m_prev;
			}
		}
	};

	Sampler& Sampler::add_ref()
	{
		if (m_sampler)
		{
			m_sampler->add_ref();
		}
		return *this;
	}

	Sampler& Sampler::release()
	{
		if (m_sampler)
		{
			m_sampler->release();
			m_sampler = nullptr;
		}
		return *this;
	}

	Sampler& Sampler::init(const RHISamplerInitializer& initializer)
	{
		release();
		m_sampler = SamplerImpl::static_find_or_create(initializer);
		return add_ref();
	}

	Sampler& Sampler::init(RHISamplerFilter filter)
	{
		release();
		m_sampler = s_default_samplers[filter.value];
		return add_ref();
	}

	const RHISamplerInitializer& Sampler::initializer() const
	{
		return m_sampler->initializer();
	}

	RHISampler* Sampler::rhi_sampler() const
	{
		if (m_sampler == nullptr)
			return nullptr;
		return m_sampler->rhi_sampler();
	}

	const Sampler& Sampler::rhi_bind(byte location) const
	{
		if (m_sampler)
		{
			if (auto rhi_sampler = m_sampler->rhi_sampler())
			{
				rhi->bind_sampler(rhi_sampler, location);
			}
		}
		return *this;
	}

	bool Sampler::serialize(Archive& ar)
	{
		byte size = m_sampler ? sizeof(RHISamplerInitializer) : 0;
		ar.serialize(size);

		if (size > 0 && ar.is_saving())
		{
			RHISamplerInitializer inititalizer = m_sampler->initializer();
			ar.serialize_memory(reinterpret_cast<byte*>(&inititalizer), size);
		}
		else if (ar.is_reading())
		{
			if (size > 0)
			{
				RHISamplerInitializer inititalizer;
				ar.serialize_memory(reinterpret_cast<byte*>(&inititalizer), size);
				init(inititalizer);
			}
			else
			{
				init(RHISamplerFilter::Point);
			}
		}

		return ar;
	}

	Sampler& Sampler::filter(RHISamplerFilter filter)
	{
		RHISamplerInitializer new_initializer = initializer();

		if (new_initializer.filter != filter)
		{
			new_initializer.filter = filter;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::address_u(RHISamplerAddressMode address)
	{
		RHISamplerInitializer new_initializer = initializer();

		if (new_initializer.address_u != address)
		{
			new_initializer.address_u = address;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::address_v(RHISamplerAddressMode address)
	{
		RHISamplerInitializer new_initializer = initializer();

		if (new_initializer.address_v != address)
		{
			new_initializer.address_v = address;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::address_w(RHISamplerAddressMode address)
	{
		RHISamplerInitializer new_initializer = initializer();

		if (new_initializer.address_w != address)
		{
			new_initializer.address_w = address;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::compare_func(RHICompareFunc func)
	{
		RHISamplerInitializer new_initializer = initializer();

		if (new_initializer.compare_func != func)
		{
			new_initializer.compare_func = func;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::border_color(const Color& color)
	{
		RHISamplerInitializer new_initializer = initializer();

		if (new_initializer.border_color != color)
		{
			new_initializer.border_color = color;
			init(new_initializer);
		}
		return *this;
	}

	RHISamplerFilter Sampler::filter() const
	{
		return initializer().filter;
	}

	RHISamplerAddressMode Sampler::address_u() const
	{
		return initializer().address_u;
	}

	RHISamplerAddressMode Sampler::address_v() const
	{
		return initializer().address_v;
	}

	RHISamplerAddressMode Sampler::address_w() const
	{
		return initializer().address_w;
	}

	RHICompareFunc Sampler::compare_func() const
	{
		return initializer().compare_func;
	}

	Color Sampler::border_color() const
	{
		return initializer().border_color;
	}

	template<RHISamplerFilter filter>
	static void construct_default_sampler()
	{
		static_assert(filter.value < s_default_samplers.size() && filter.value >= 0);

		RHISamplerInitializer initializer;
		initializer.filter   = filter;
		SamplerImpl* sampler = trx_new SamplerImpl(initializer);
		sampler->add_ref();
		s_default_samplers[filter.value] = sampler;
	}

	static void construct_default_samplers()
	{
		construct_default_sampler<RHISamplerFilter::Point>();
		construct_default_sampler<RHISamplerFilter::Bilinear>();
		construct_default_sampler<RHISamplerFilter::Trilinear>();
	}

	static void initialize_default_samplers()
	{
		for (SamplerImpl* sampler : s_default_samplers)
		{
			if (sampler)
			{
				sampler->initialize();
			}
		}
	}

	static void terminate_samplers()
	{
		s_gc_disabled = true;
		for (SamplerImpl* sampler : s_default_samplers) sampler->release();

		SamplerImpl* sampler = s_first_sampler;

		while (sampler)
		{
			if (sampler->references() == 0)
			{
				SamplerImpl* next = sampler->next();
				trx_delete sampler;
				sampler = next;
			}
			else
			{
				sampler = sampler->next();
			}
		}
	}

	static PreInitializeController preinitializer(construct_default_samplers);
	static StartupResourcesInitializeController initializer(initialize_default_samplers);
	static DestroyController terminate(terminate_samplers);
}// namespace Engine
