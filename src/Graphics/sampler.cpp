#include <Core/archive.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/array.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/map.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/threading.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <limits>

namespace Engine
{
	trinex_implement_struct(Engine::Sampler, 0)
	{
		auto self = static_struct_instance();
		trinex_refl_virtual_prop(self, filter, filter, filter, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(self, address_u, address_u, address_u, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(self, address_v, address_v, address_v, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(self, address_w, address_w, address_w, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(self, compare_mode, compare_mode, compare_mode, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(self, compare_func, compare_func, compare_func, Refl::Property::IsTransient);
		trinex_refl_virtual_prop(self, border_color, border_color, border_color, Refl::Property::IsTransient);
	}

	struct SamplerInitializerHash {
		inline size_t operator()(const SamplerInitializer& initializer) const { return initializer.hash(); }
	};

	using SamplersMap = Map<SamplerInitializer, class SamplerImpl*, SamplerInitializerHash>;

	static SamplersMap s_samplers;
	static Array<SamplerImpl*, 3> s_default_samplers;
	static SamplerImpl* s_first_sampler = nullptr;
	static SamplerImpl* s_last_sampler  = nullptr;
	static bool s_gc_disabled           = false;

	class SamplerImpl
	{
	private:
		Atomic<uint64_t> m_references = 0;
		RHI_Sampler* m_sampler        = nullptr;
		SamplerImpl* m_prev           = nullptr;
		SamplerImpl* m_next           = nullptr;
		SamplerInitializer m_initializer;

	public:
		SamplerImpl(const SamplerInitializer& initializer) : m_initializer(initializer)
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
				Engine::release(this);
			}
		}

		inline uint64_t references() const { return m_references; }
		inline const SamplerInitializer& initializer() const { return m_initializer; }
		inline RHI_Sampler* rhi_sampler() const { return m_sampler; }
		inline SamplerImpl* prev() const { return m_prev; }
		inline SamplerImpl* next() const { return m_next; }

		static inline SamplerImpl* static_find_or_create(const SamplerInitializer& initializer)
		{
			auto it = s_samplers.find(initializer);

			if (it == s_samplers.end())
				return &allocate<SamplerImpl>(initializer)->initialize();

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

	SamplerInitializer::SamplerInitializer()
	    : filter(SamplerFilter::Point),             //
	      address_u(SamplerAddressMode::Repeat),    //
	      address_v(SamplerAddressMode::Repeat),    //
	      address_w(SamplerAddressMode::Repeat),    //
	      compare_mode(CompareMode::None),          //
	      compare_func(CompareFunc::Always),        //
	      border_color(0, 0, 0, 255),               //
	      anisotropy(1.0),                          //
	      mip_lod_bias(0.0),                        //
	      min_lod(0.f),                             //
	      max_lod(std::numeric_limits<float>::max())//
	{}

	HashIndex SamplerInitializer::hash() const
	{
		return memory_hash(this, sizeof(SamplerInitializer));
	}

	bool SamplerInitializer::operator==(const SamplerInitializer& initializer) const
	{
		return filter == initializer.filter &&            //
		       address_u == initializer.address_u &&      //
		       address_v == initializer.address_v &&      //
		       address_w == initializer.address_w &&      //
		       compare_mode == initializer.compare_mode &&//
		       compare_func == initializer.compare_func &&//
		       border_color == initializer.border_color &&//
		       anisotropy == initializer.anisotropy &&    //
		       mip_lod_bias == initializer.mip_lod_bias &&//
		       min_lod == initializer.min_lod &&          //
		       max_lod == initializer.max_lod;
	}

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

	Sampler& Sampler::init(const SamplerInitializer& initializer)
	{
		release();
		m_sampler = SamplerImpl::static_find_or_create(initializer);
		return add_ref();
	}

	Sampler& Sampler::init(SamplerFilter filter)
	{
		release();
		m_sampler = s_default_samplers[filter.value];
		return add_ref();
	}

	const SamplerInitializer& Sampler::initializer() const
	{
		return m_sampler->initializer();
	}

	RHI_Sampler* Sampler::rhi_sampler() const
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
				rhi_sampler->bind(location);
			}
		}
		return *this;
	}

	bool Sampler::serialize(Archive& ar)
	{
		byte size = m_sampler ? sizeof(SamplerInitializer) : 0;
		ar.serialize(size);

		if (size > 0 && ar.is_saving())
		{
			SamplerInitializer inititalizer = m_sampler->initializer();
			ar.serialize_memory(reinterpret_cast<byte*>(&inititalizer), size);
		}
		else if (ar.is_reading())
		{
			if (size > 0)
			{
				SamplerInitializer inititalizer;
				ar.serialize_memory(reinterpret_cast<byte*>(&inititalizer), size);
				init(inititalizer);
			}
			else
			{
				init(SamplerFilter::Point);
			}
		}

		return ar;
	}

	Sampler& Sampler::filter(SamplerFilter filter)
	{
		SamplerInitializer new_initializer = initializer();

		if (new_initializer.filter != filter)
		{
			new_initializer.filter = filter;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::address_u(SamplerAddressMode address)
	{
		SamplerInitializer new_initializer = initializer();

		if (new_initializer.address_u != address)
		{
			new_initializer.address_u = address;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::address_v(SamplerAddressMode address)
	{
		SamplerInitializer new_initializer = initializer();

		if (new_initializer.address_v != address)
		{
			new_initializer.address_v = address;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::address_w(SamplerAddressMode address)
	{
		SamplerInitializer new_initializer = initializer();

		if (new_initializer.address_w != address)
		{
			new_initializer.address_w = address;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::compare_mode(CompareMode mode)
	{
		SamplerInitializer new_initializer = initializer();

		if (new_initializer.compare_mode != mode)
		{
			new_initializer.compare_mode = mode;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::compare_func(CompareFunc func)
	{
		SamplerInitializer new_initializer = initializer();

		if (new_initializer.compare_func != func)
		{
			new_initializer.compare_func = func;
			init(new_initializer);
		}
		return *this;
	}

	Sampler& Sampler::border_color(const Color& color)
	{
		SamplerInitializer new_initializer = initializer();

		if (new_initializer.border_color != color)
		{
			new_initializer.border_color = color;
			init(new_initializer);
		}
		return *this;
	}

	template<SamplerFilter filter>
	static void construct_default_sampler()
	{
		static_assert(filter.value < s_default_samplers.size() && filter.value >= 0);

		SamplerInitializer initializer;
		initializer.filter   = filter;
		SamplerImpl* sampler = allocate<SamplerImpl>(initializer);
		sampler->add_ref();
		s_default_samplers[filter.value] = sampler;
	}

	static void construct_default_samplers()
	{
		construct_default_sampler<SamplerFilter::Point>();
		construct_default_sampler<SamplerFilter::Bilinear>();
		construct_default_sampler<SamplerFilter::Trilinear>();
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
				release(sampler);
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
