#include <Core/base_engine.hpp>
#include <Core/etl/vector.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/file.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/math/math.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Core/types/path.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <Input/input_system.hpp>
#include <RHI/context.hpp>
#include <RHI/initializers.hpp>
#include <RHI/resource_ptr.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <UI/rml.hpp>
#include <Window/window.hpp>

namespace Trinex::UI
{
	namespace
	{
		static bool& rml_debugger_initialized()
		{
			static bool initialized = false;
			return initialized;
		}

		static void collect_controllers(Map<RML::Element*, RMLController*>& controllers, RML::Element* element, RMLClient* owner)
		{
			if (element == nullptr || owner == nullptr)
				return;

			String controller_name = element->GetAttribute<String>("data-controller", "");

			if (!controller_name.empty())
			{
				Refl::Class* controller_class = Refl::Class::static_find<Refl::Class>(controller_name);

				if (controller_class == nullptr && !controller_name.starts_with("Trinex::"))
				{
					controller_class = Refl::Class::static_find<Refl::Class>(Strings::format("Trinex::UI::{}", controller_name));
				}

				if (controller_class && controller_class->is_a<RMLController>())
				{
					if (auto controller = Object::instance_cast<RMLController>(controller_class->create_object("", owner)))
					{
						controller->attach(element);
						controllers.insert({element, controller});
					}
				}
				else
				{
					error_log("RML", "Failed to find controller '%s' for element id '%s'", controller_name.c_str(),
					          element->GetId().c_str());
				}
			}

			for (int i = 0, count = element->GetNumChildren(); i < count; ++i)
			{
				collect_controllers(controllers, element->GetChild(i), owner);
			}
		}

		static DeviceId window_keyboard_device_id(Identifier window_id)
		{
			return (static_cast<DeviceId>(InputDeviceType::Keyboard) << 56) | static_cast<DeviceId>(window_id);
		}

		static RML::Input::KeyIdentifier map_key_identifier(KeyCode key_code)
		{
#define TRINEX_RML_KEY_CASE(engine_key, rml_key)                                                                                 \
	case KeyCode::engine_key: return RML::Input::KI_##rml_key;
			switch (key_code)
			{
				TRINEX_RML_KEY_CASE(Undefined, UNKNOWN)
				TRINEX_RML_KEY_CASE(Space, SPACE)
				TRINEX_RML_KEY_CASE(Num0, 0)
				TRINEX_RML_KEY_CASE(Num1, 1)
				TRINEX_RML_KEY_CASE(Num2, 2)
				TRINEX_RML_KEY_CASE(Num3, 3)
				TRINEX_RML_KEY_CASE(Num4, 4)
				TRINEX_RML_KEY_CASE(Num5, 5)
				TRINEX_RML_KEY_CASE(Num6, 6)
				TRINEX_RML_KEY_CASE(Num7, 7)
				TRINEX_RML_KEY_CASE(Num8, 8)
				TRINEX_RML_KEY_CASE(Num9, 9)
				TRINEX_RML_KEY_CASE(A, A)
				TRINEX_RML_KEY_CASE(B, B)
				TRINEX_RML_KEY_CASE(C, C)
				TRINEX_RML_KEY_CASE(D, D)
				TRINEX_RML_KEY_CASE(E, E)
				TRINEX_RML_KEY_CASE(F, F)
				TRINEX_RML_KEY_CASE(G, G)
				TRINEX_RML_KEY_CASE(H, H)
				TRINEX_RML_KEY_CASE(I, I)
				TRINEX_RML_KEY_CASE(J, J)
				TRINEX_RML_KEY_CASE(K, K)
				TRINEX_RML_KEY_CASE(L, L)
				TRINEX_RML_KEY_CASE(M, M)
				TRINEX_RML_KEY_CASE(N, N)
				TRINEX_RML_KEY_CASE(O, O)
				TRINEX_RML_KEY_CASE(P, P)
				TRINEX_RML_KEY_CASE(Q, Q)
				TRINEX_RML_KEY_CASE(R, R)
				TRINEX_RML_KEY_CASE(S, S)
				TRINEX_RML_KEY_CASE(T, T)
				TRINEX_RML_KEY_CASE(U, U)
				TRINEX_RML_KEY_CASE(V, V)
				TRINEX_RML_KEY_CASE(W, W)
				TRINEX_RML_KEY_CASE(X, X)
				TRINEX_RML_KEY_CASE(Y, Y)
				TRINEX_RML_KEY_CASE(Z, Z)
				TRINEX_RML_KEY_CASE(Backspace, BACK)
				TRINEX_RML_KEY_CASE(Tab, TAB)
				TRINEX_RML_KEY_CASE(Return, RETURN)
				TRINEX_RML_KEY_CASE(Escape, ESCAPE)
				TRINEX_RML_KEY_CASE(CapsLock, CAPITAL)
				TRINEX_RML_KEY_CASE(Insert, INSERT)
				TRINEX_RML_KEY_CASE(Delete, DELETE)
				TRINEX_RML_KEY_CASE(Home, HOME)
				TRINEX_RML_KEY_CASE(End, END)
				TRINEX_RML_KEY_CASE(PageUp, PRIOR)
				TRINEX_RML_KEY_CASE(PageDown, NEXT)
				TRINEX_RML_KEY_CASE(Left, LEFT)
				TRINEX_RML_KEY_CASE(Right, RIGHT)
				TRINEX_RML_KEY_CASE(Up, UP)
				TRINEX_RML_KEY_CASE(Down, DOWN)
				TRINEX_RML_KEY_CASE(F1, F1)
				TRINEX_RML_KEY_CASE(F2, F2)
				TRINEX_RML_KEY_CASE(F3, F3)
				TRINEX_RML_KEY_CASE(F4, F4)
				TRINEX_RML_KEY_CASE(F5, F5)
				TRINEX_RML_KEY_CASE(F6, F6)
				TRINEX_RML_KEY_CASE(F7, F7)
				TRINEX_RML_KEY_CASE(F8, F8)
				TRINEX_RML_KEY_CASE(F9, F9)
				TRINEX_RML_KEY_CASE(F10, F10)
				TRINEX_RML_KEY_CASE(F11, F11)
				TRINEX_RML_KEY_CASE(F12, F12)
				TRINEX_RML_KEY_CASE(F13, F13)
				TRINEX_RML_KEY_CASE(F14, F14)
				TRINEX_RML_KEY_CASE(F15, F15)
				TRINEX_RML_KEY_CASE(F16, F16)
				TRINEX_RML_KEY_CASE(F17, F17)
				TRINEX_RML_KEY_CASE(F18, F18)
				TRINEX_RML_KEY_CASE(F19, F19)
				TRINEX_RML_KEY_CASE(F20, F20)
				TRINEX_RML_KEY_CASE(F21, F21)
				TRINEX_RML_KEY_CASE(F22, F22)
				TRINEX_RML_KEY_CASE(F23, F23)
				TRINEX_RML_KEY_CASE(F24, F24)
				TRINEX_RML_KEY_CASE(PrintScreen, SNAPSHOT)
				TRINEX_RML_KEY_CASE(ScrollLock, SCROLL)
				TRINEX_RML_KEY_CASE(Pause, PAUSE)
				TRINEX_RML_KEY_CASE(NumLockClear, NUMLOCK)
				TRINEX_RML_KEY_CASE(KpDivide, DIVIDE)
				TRINEX_RML_KEY_CASE(KpMultiply, MULTIPLY)
				TRINEX_RML_KEY_CASE(KpMinus, SUBTRACT)
				TRINEX_RML_KEY_CASE(KpPlus, ADD)
				TRINEX_RML_KEY_CASE(KpEnter, NUMPADENTER)
				TRINEX_RML_KEY_CASE(Kp1, NUMPAD1)
				TRINEX_RML_KEY_CASE(Kp2, NUMPAD2)
				TRINEX_RML_KEY_CASE(Kp3, NUMPAD3)
				TRINEX_RML_KEY_CASE(Kp4, NUMPAD4)
				TRINEX_RML_KEY_CASE(Kp5, NUMPAD5)
				TRINEX_RML_KEY_CASE(Kp6, NUMPAD6)
				TRINEX_RML_KEY_CASE(Kp7, NUMPAD7)
				TRINEX_RML_KEY_CASE(Kp8, NUMPAD8)
				TRINEX_RML_KEY_CASE(Kp9, NUMPAD9)
				TRINEX_RML_KEY_CASE(Kp0, NUMPAD0)
				TRINEX_RML_KEY_CASE(KpPeriod, DECIMAL)
				TRINEX_RML_KEY_CASE(KpEquals, OEM_NEC_EQUAL)
				TRINEX_RML_KEY_CASE(LeftControl, LCONTROL)
				TRINEX_RML_KEY_CASE(LeftShift, LSHIFT)
				TRINEX_RML_KEY_CASE(LeftAlt, LMENU)
				TRINEX_RML_KEY_CASE(LeftGui, LMETA)
				TRINEX_RML_KEY_CASE(RightControl, RCONTROL)
				TRINEX_RML_KEY_CASE(RightShift, RSHIFT)
				TRINEX_RML_KEY_CASE(RightAlt, RMENU)
				TRINEX_RML_KEY_CASE(RightGui, RMETA)
				TRINEX_RML_KEY_CASE(Application, APPS)
				TRINEX_RML_KEY_CASE(Menu, APPS)
				TRINEX_RML_KEY_CASE(Select, SELECT)
				TRINEX_RML_KEY_CASE(Execute, EXECUTE)
				TRINEX_RML_KEY_CASE(Help, HELP)
				TRINEX_RML_KEY_CASE(Mode, MODECHANGE)
				TRINEX_RML_KEY_CASE(Cancel, ESCAPE)
				TRINEX_RML_KEY_CASE(Clear, CLEAR)
				TRINEX_RML_KEY_CASE(Return2, RETURN)
				TRINEX_RML_KEY_CASE(Separator, SEPARATOR)
				TRINEX_RML_KEY_CASE(Out, UNKNOWN)
				TRINEX_RML_KEY_CASE(Oper, UNKNOWN)
				TRINEX_RML_KEY_CASE(ClearAgain, CLEAR)
				TRINEX_RML_KEY_CASE(CrSel, CRSEL)
				TRINEX_RML_KEY_CASE(ExSel, EXSEL)
				TRINEX_RML_KEY_CASE(Mute, VOLUME_MUTE)
				TRINEX_RML_KEY_CASE(VolumeUp, VOLUME_UP)
				TRINEX_RML_KEY_CASE(VolumeDown, VOLUME_DOWN)
				TRINEX_RML_KEY_CASE(AudioNext, MEDIA_NEXT_TRACK)
				TRINEX_RML_KEY_CASE(AudioPrev, MEDIA_PREV_TRACK)
				TRINEX_RML_KEY_CASE(AudioStop, MEDIA_STOP)
				TRINEX_RML_KEY_CASE(AudioPlay, MEDIA_PLAY_PAUSE)
				TRINEX_RML_KEY_CASE(AudioMute, VOLUME_MUTE)
				TRINEX_RML_KEY_CASE(MediaSelect, LAUNCH_MEDIA_SELECT)
				TRINEX_RML_KEY_CASE(Mail, LAUNCH_MAIL)
				TRINEX_RML_KEY_CASE(Computer, LAUNCH_APP1)
				TRINEX_RML_KEY_CASE(AcSearch, BROWSER_SEARCH)
				TRINEX_RML_KEY_CASE(AcHome, BROWSER_HOME)
				TRINEX_RML_KEY_CASE(AcBack, BROWSER_BACK)
				TRINEX_RML_KEY_CASE(AcForward, BROWSER_FORWARD)
				TRINEX_RML_KEY_CASE(AcStop, BROWSER_STOP)
				TRINEX_RML_KEY_CASE(AcRefresh, BROWSER_REFRESH)
				TRINEX_RML_KEY_CASE(AcBookmarks, BROWSER_FAVORITES)
				TRINEX_RML_KEY_CASE(BrightnessDown, UNKNOWN)
				TRINEX_RML_KEY_CASE(BrightnessUp, UNKNOWN)
				TRINEX_RML_KEY_CASE(DisplaySwitch, UNKNOWN)
				TRINEX_RML_KEY_CASE(KbdIllumToggle, UNKNOWN)
				TRINEX_RML_KEY_CASE(KbdIllumDown, UNKNOWN)
				TRINEX_RML_KEY_CASE(KbdIllumUp, UNKNOWN)
				default: return RML::Input::KI_UNKNOWN;
			}
#undef TRINEX_RML_KEY_CASE
		}

		static int key_modifier_state(DeviceId device_id, Identifier fallback_window_id = 0)
		{
			const InputSystem* input = InputSystem::instance();
			if (!input)
				return 0;

			const InputDeviceState* keyboard = input->keyboard_state(device_id);
			if (!keyboard)
			{
				keyboard = input->keyboard_state_for_user(0);
			}

			if (!keyboard && fallback_window_id != 0)
			{
				keyboard = input->keyboard_state(window_keyboard_device_id(fallback_window_id));
			}

			if (!keyboard)
				return 0;

			const auto pressed = [keyboard](KeyCode key_code) { return keyboard->keyboard.keys[static_cast<usize>(key_code)]; };

			int state = 0;
			if (pressed(KeyCode::LeftControl) || pressed(KeyCode::RightControl))
				state |= RML::Input::KM_CTRL;
			if (pressed(KeyCode::LeftShift) || pressed(KeyCode::RightShift))
				state |= RML::Input::KM_SHIFT;
			if (pressed(KeyCode::LeftAlt) || pressed(KeyCode::RightAlt))
				state |= RML::Input::KM_ALT;
			if (pressed(KeyCode::LeftGui) || pressed(KeyCode::RightGui))
				state |= RML::Input::KM_META;

			return state;
		}

		static int mouse_button_index(MouseButton button)
		{
			switch (button)
			{
				case MouseButton::Left: return 0;
				case MouseButton::Right: return 1;
				case MouseButton::Middle: return 2;
				case MouseButton::X1: return 3;
				case MouseButton::X2: return 4;
				default: return -1;
			}
		}

		class RMLColorPipeline : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(RMLColorPipeline, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_translate = nullptr;
			const RHIShaderParameterInfo* m_transform = nullptr;

		public:
			RMLColorPipeline& setup(RHIContext* context, const Matrix4f& transform, Vector2f translate)
			{
				context->bind_pipeline(handle());
				context->update_scalar(&translate, m_translate);
				context->update_scalar(&transform, m_transform);
				return *this;
			}
		};

		trinex_implement_pipeline(RMLColorPipeline, "[shaders]:/TrinexEditor/rml/color.slang")
		{
			m_translate = find_parameter("translate");
			m_transform = find_parameter("transform");
		}

		class RMLTexturePipeline : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(RMLTexturePipeline, GlobalPipelineLibrary);

			const RHIShaderParameterInfo* m_translate = nullptr;
			const RHIShaderParameterInfo* m_transform = nullptr;
			const RHIShaderParameterInfo* m_texture   = nullptr;

		public:
			RMLTexturePipeline& setup(RHIContext* context, const Matrix4f& transform, Vector2f translate, RHITexture* texture,
			                          RHISampler* sampler)
			{
				context->bind_pipeline(handle());
				context->update_scalar(&translate, m_translate);
				context->update_scalar(&transform, m_transform);
				context->bind_srv(texture->as_srv(), m_texture->binding);
				context->bind_sampler(RHIBilinearWrapSampler::static_sampler(), m_texture->binding);
				return *this;
			}
		};

		trinex_implement_pipeline(RMLTexturePipeline, "[shaders]:/TrinexEditor/rml/texture.slang")
		{
			m_translate = find_parameter("translate");
			m_transform = find_parameter("transform");
			m_texture   = find_parameter("texture");
		}

		class RMLGeometry
		{
		public:
			RHIResourcePtr<RHIBuffer> vertex;
			RHIResourcePtr<RHIBuffer> index;

			static RMLGeometry* from(RML::CompiledGeometryHandle handle) { return reinterpret_cast<RMLGeometry*>(handle); }
			inline RML::CompiledGeometryHandle handle() const { return reinterpret_cast<RML::CompiledGeometryHandle>(this); }
		};

		class RMLTexture
		{
		public:
			virtual RHISampler* sampler() = 0;
			virtual RHITexture* texture() = 0;
			virtual ~RMLTexture() {}

			static RMLTexture* from(RML::CompiledGeometryHandle handle) { return reinterpret_cast<RMLTexture*>(handle); }
			inline RML::CompiledGeometryHandle handle() const { return reinterpret_cast<RML::CompiledGeometryHandle>(this); }
		};

		class RMLTextureCopy final : public RMLTexture
		{
		private:
			RHITexture* m_texture;
			RHISampler* m_sampler;

		public:
			RMLTextureCopy(Vector2u size, RHITextureFlags flags, RHISampler* sampler = nullptr)
			{
				auto pool = RHITexturePool::global_instance();
				m_texture = pool->acquire(RHISurfaceFormat::RGBA8, {size.x, size.y}, flags);
				m_sampler = sampler ? sampler : RHIBilinearWrapSampler::static_sampler();
			}

			RHISampler* sampler() override { return m_sampler; }
			RHITexture* texture() override { return m_texture; }

			~RMLTextureCopy()
			{
				auto pool = RHITexturePool::global_instance();
				pool->release(reinterpret_cast<RHITexture*>(m_texture));
			}
		};

		class RMLTextureRef final : public RMLTexture
		{
		private:
			RHITexture* m_texture;
			RHISampler* m_sampler;

		public:
			RMLTextureRef(RHITexture* texture, RHISampler* sampler = nullptr) : m_texture(texture)
			{
				texture->add_reference();
				m_sampler = sampler ? sampler : RHIBilinearWrapSampler::static_sampler();
			}

			RHISampler* sampler() override { return m_sampler; }
			RHITexture* texture() override { return m_texture; }
			~RMLTextureRef() { m_texture->release(); }
		};

		class RMLRenderInterface final : public RML::RenderInterface
		{
		private:
			struct Flags {
				enum Enum : u8
				{
					Undefined           = 0,
					IsInRendering       = BIT(0),
					IsDepthStencilDirty = BIT(1),
					IsBlendingDirty     = BIT(2),

					Default = IsDepthStencilDirty | IsBlendingDirty,
				};

				trinex_bitfield_enum_struct(Flags, u8);
			};

			RHIContext* m_context = nullptr;
			RHITexture* m_depth   = nullptr;
			Vector<RHITexture*> m_layers;

			RHIDepthStencilState m_depth_stencil;
			RHIBlendingState m_blending;
			RHIRegion m_scissor;

			Vector2u m_size;
			Matrix4f m_projection = Matrix4f(1.f);
			Matrix4f m_transform  = Matrix4f(1.f);

			u64 m_layer   = 0;
			Flags m_flags = Flags::Undefined;

			template<typename T>
			static inline usize DataSize(const RML::Span<T>& data)
			{
				return data.size() * sizeof(T);
			}

			static inline Vector2f AsVector(RML::Vector2f vector) { return {vector.x, vector.y}; }
			static inline Vector2i AsVector(RML::Vector2i vector) { return {vector.x, vector.y}; }

			static RHITexture* RequestTexture(Vector2u size, RHITextureFlags flags)
			{
				auto pool = RHITexturePool::global_instance();
				return pool->acquire(RHISurfaceFormat::RGBA8, {size.x, size.y}, flags);
			}

		public:
			void BeginFrame(RHISwapchain* swapchain)
			{
				auto pool = RHITexturePool::global_instance();

				m_size = swapchain->as_texture()->size();
				m_layers.push_back(pool->acquire(RHISurfaceFormat::RGBA8, m_size, RHITextureFlags::ColorAttachment));
				m_depth = pool->acquire(RHISurfaceFormat::D24S8, m_size, RHITextureFlags::DepthStencilAttachment);

				m_projection = Math::ortho(0, m_size.x, m_size.y, 0, 0.f, 1.f);
				m_transform  = m_projection;

				m_layer = 0;
				m_flags = Flags::Default;

				m_context = RHIContextPool::global_instance()->begin();

				// clang-format off
				m_context->bind_vertex_attribute(RHISemantic::Position, RHIVertexFormat::RG32F, 0, offsetof(RML::Vertex, position));
				m_context->bind_vertex_attribute(RHISemantic::Color, RHIVertexFormat::RGBA8, 0, offsetof(RML::Vertex, colour));
				m_context->bind_vertex_attribute(RHISemantic::TexCoord0, RHIVertexFormat::RG32F, 0, offsetof(RML::Vertex, tex_coord));
				// clang-format on

				// Setup state
				m_blending      = RHIBlendingState::alpha_composite();
				m_depth_stencil = RHIDepthStencilState();
				m_scissor       = RHIRegion();

				m_context->push_debug_stage("RML Scene");

				m_context->barrier(m_layers[0], RHIAccess::TransferDst);
				m_context->barrier(m_depth, RHIAccess::TransferDst);

				m_context->clear_rtv(m_layers[0]->as_rtv());
				m_context->clear_dsv(m_depth->as_dsv());

				m_context->barrier(m_layers[0], RHIAccess::RTV);
				m_context->barrier(m_depth, RHIAccess::DSV);
			}

			void EndFrame(RHISwapchain* swapchain)
			{
				EndRendering();

				m_context->push_debug_stage("Copy to swapchain");
				{
					m_context->barrier(swapchain->as_texture(), RHIAccess::TransferDst);
					m_context->barrier(m_layers[0], RHIAccess::TransferSrc);

					m_context->copy(swapchain->as_texture(), m_layers[0], RHITextureRegion(m_size));
					m_context->barrier(swapchain->as_texture(), RHIAccess::PresentSrc);
				}
				m_context->pop_debug_stage();
				m_context->pop_debug_stage();

				auto wait   = swapchain->acquire_semaphore();
				auto signal = swapchain->present_semaphore();

				RHIContextPool::global_instance()->end(m_context, wait, signal);
				RHI::instance()->present(swapchain);

				// Release all transient resources
				auto pool = RHITexturePool::global_instance();

				for (RHITexture* layer : m_layers)
				{
					pool->release(layer);
				}

				pool->release(m_depth);
				m_layers.clear();
			}

			void BeginRendering()
			{
				if (!m_flags.all(Flags::IsInRendering))
				{
					m_context->barrier(m_layers[m_layer], RHIAccess::RTV);
					m_context->barrier(m_depth, RHIAccess::DSV);

					m_context->begin_rendering(RHIRenderingInfo(m_layers[m_layer]->as_rtv(), m_depth->as_dsv()));
					m_flags.set(Flags::IsInRendering);
				}
			}

			void EndRendering()
			{
				if (m_flags.all(Flags::IsInRendering))
				{
					m_context->end_rendering();
					m_flags.remove(Flags::IsInRendering);
				}
			}

			inline bool IsRendering() const { return m_flags.all(Flags::IsInRendering); }
			inline Vector2u GetRenderSize() const { return m_size; }
			inline RHITexture* GetRenderTarget() const { return m_layer < m_layers.size() ? m_layers[m_layer] : nullptr; }
			inline RHIContext* GetContext() const { return m_context; }

			void RenderTexture(RHITexture* texture, const Vector2f& position, const Vector2f& size)
			{
				if (texture == nullptr || size.x <= 0.f || size.y <= 0.f)
					return;

				m_context->barrier(texture, RHIAccess::SRVGraphics);
				BeginRendering();
				const Vector2f texel = 1.f / Vector2f(m_size);
				Pipelines::Passthrow::passthrow(m_context, RHIRegion(size * texel, position * texel), texture->as_srv());
				m_flags.set(Flags::IsDepthStencilDirty | Flags::IsBlendingDirty);
			}

			RML::CompiledGeometryHandle CompileGeometry(RML::Span<const RML::Vertex> vertices,
			                                            RML::Span<const int> indices) override
			{
				RMLGeometry* geometry = trx_new RMLGeometry();
				geometry->vertex      = RHI::instance()->create_buffer(DataSize(vertices), RHIBufferFlags::VertexBuffer);
				geometry->index       = RHI::instance()->create_buffer(DataSize(indices), RHIBufferFlags::IndexBuffer);

				EndRendering();

				m_context->barrier(geometry->vertex, RHIAccess::TransferDst);
				m_context->barrier(geometry->index, RHIAccess::TransferDst);

				m_context->update(geometry->vertex, vertices.data(), {.size = DataSize(vertices)});
				m_context->update(geometry->index, indices.data(), {.size = DataSize(indices)});

				m_context->barrier(geometry->vertex, RHIAccess::VertexBuffer);
				m_context->barrier(geometry->index, RHIAccess::IndexBuffer);

				return geometry->handle();
			}

			void RenderGeometry(RML::CompiledGeometryHandle handle, RML::Vector2f translation,
			                    RML::TextureHandle texture) override
			{
				BeginRendering();

				if (m_flags.all(Flags::IsDepthStencilDirty))
					m_context->depth_stencil_state(m_depth_stencil);

				if (m_flags.all(Flags::IsBlendingDirty))
					m_context->blending_state(m_blending);

				m_flags.remove(Flags::IsDepthStencilDirty | Flags::IsBlendingDirty);

				if (texture)
				{
					auto handle = RMLTexture::from(texture);
					RMLTexturePipeline::instance()->setup(m_context, m_transform, {translation.x, translation.y},
					                                      handle->texture(), handle->sampler());
				}
				else
				{
					RMLColorPipeline::instance()->setup(m_context, m_transform, {translation.x, translation.y});
				}

				RMLGeometry* geometry = RMLGeometry::from(handle);
				m_context->bind_vertex_buffer(geometry->vertex, 0, sizeof(RML::Vertex), 0);
				m_context->bind_index_buffer(geometry->index, RHIIndexFormat::UInt32, 0);
				m_context->draw_indexed(RHITopology::TriangleList, geometry->index->size() / 4, 0, 0);
			}

			void ReleaseGeometry(RML::CompiledGeometryHandle geometry) override { trx_delete RMLGeometry::from(geometry); }

			RML::TextureHandle LoadTexture(RML::Vector2i& size, const RML::String& source) override
			{
				/*
				 	# = RHI texture
					@ = Engine texture
					$ = Content texture path
					! = File texture path
				 */

				if (source.empty())
					return 0;


				switch (source[0])
				{
					case '#':
					{
						auto handle       = static_cast<RHITexture*>(Strings::pointer_of(source.data() + 1));
						auto texture      = trx_new RMLTextureRef(handle);
						auto texture_size = handle->size();
						size              = RML::Vector2i(texture_size.x, texture_size.y);
						return texture->handle();
					}

					case '@':
					{
						auto handle       = static_cast<Texture*>(Strings::pointer_of(source.data() + 1))->handle();
						auto texture      = trx_new RMLTextureRef(handle);
						auto texture_size = texture->texture()->size();
						size              = RML::Vector2i(texture_size.x, texture_size.y);
						return texture->handle();
					}

					case '$':
					{
						return 0;
					}

					default:
					{
						return 0;
					}
				}

				return 0;
			}

			RML::TextureHandle GenerateTexture(RML::Span<const RML::byte> source, RML::Vector2i size) override
			{
				auto texture = trx_new RMLTextureCopy({size.x, size.y}, RHITextureFlags::ShaderResource);

				RHITextureRegion dst = RHITextureRegion({size.x, size.y, 1});
				EndRendering();

				m_context->barrier(texture->texture(), RHIAccess::TransferDst);
				m_context->update(texture->texture(), dst, source.data(), {.size = source.size()});
				m_context->barrier(texture->texture(), RHIAccess::SRVGraphics);
				return texture->handle();
			}

			void ReleaseTexture(RML::TextureHandle texture) override { trx_delete RMLTexture::from(texture); }

			void EnableScissorRegion(bool enable) override
			{
				if (!enable)
				{
					m_context->scissor(RHIRegion());
					m_scissor = RHIRegion();
				}
			}

			void SetScissorRegion(RML::Rectanglei region) override
			{
				m_scissor.pos  = AsVector(RML::Vector2f(region.Position())) / Vector2f(m_size);
				m_scissor.size = AsVector(RML::Vector2f(region.Size())) / Vector2f(m_size);
				m_context->scissor(m_scissor);
			}

			void EnableClipMask(bool enable) override
			{
				m_depth_stencil.stencil.compare = enable ? RHICompareFunc::Equal : RHICompareFunc::Always;
				m_flags.set(Flags::IsDepthStencilDirty);
			}

			void RenderToClipMask(RML::ClipMaskOperation operation, RML::CompiledGeometryHandle geometry,
			                      RML::Vector2f translation) override
			{
				u32 stencil_write_value = 1;
				u32 stencil_test_value  = 1;

				switch (operation)
				{
					case RML::ClipMaskOperation::Set:
					{
						EndRendering();

						// @performance Increment the reference value instead of clearing each time.
						m_context->barrier(m_depth, RHIAccess::TransferDst);
						m_context->clear_dsv(m_depth->as_dsv(), RHIAspect::Stencil);
						m_context->barrier(m_depth, RHIAccess::DSV);

						m_depth_stencil.stencil.fail       = RHIStencilOp::Keep;
						m_depth_stencil.stencil.depth_fail = RHIStencilOp::Keep;
						m_depth_stencil.stencil.depth_pass = RHIStencilOp::Replace;
						break;
					}

					case RML::ClipMaskOperation::SetInverse:
					{
						EndRendering();

						m_context->barrier(m_depth, RHIAccess::TransferDst);
						m_context->clear_dsv(m_depth->as_dsv(), RHIAspect::Stencil, 0.f, 1);
						m_context->barrier(m_depth, RHIAccess::DSV);

						m_depth_stencil.stencil.fail       = RHIStencilOp::Keep;
						m_depth_stencil.stencil.depth_fail = RHIStencilOp::Keep;
						m_depth_stencil.stencil.depth_pass = RHIStencilOp::Replace;
						stencil_write_value                = 0;
						break;
					}

					case RML::ClipMaskOperation::Intersect:
					{
						m_depth_stencil.stencil.fail       = RHIStencilOp::Keep;
						m_depth_stencil.stencil.depth_fail = RHIStencilOp::Keep;
						m_depth_stencil.stencil.depth_pass = RHIStencilOp::Incr;

						stencil_test_value = m_depth_stencil.stencil.reference + 1;
						break;
					}
				}

				m_blending.write_mask             = RHIColorComponent::Undefined;
				m_depth_stencil.stencil.compare   = RHICompareFunc::Always;
				m_depth_stencil.stencil.reference = stencil_write_value;
				m_flags.set(Flags::IsDepthStencilDirty | Flags::IsBlendingDirty);

				m_context->push_debug_stage("Clip Mask Rendering");
				RenderGeometry(geometry, translation, {});
				m_context->pop_debug_stage();

				m_blending.write_mask              = RHIColorComponent::RGBA;
				m_depth_stencil.stencil.compare    = RHICompareFunc::Equal;
				m_depth_stencil.stencil.reference  = stencil_test_value;
				m_depth_stencil.stencil.fail       = RHIStencilOp::Keep;
				m_depth_stencil.stencil.depth_fail = RHIStencilOp::Keep;
				m_depth_stencil.stencil.depth_pass = RHIStencilOp::Keep;
				m_flags.set(Flags::IsDepthStencilDirty | Flags::IsBlendingDirty);
			}

			void SetTransform(const RML::Matrix4f* transform) override
			{
				m_transform = (transform ? (m_projection * (*reinterpret_cast<const Matrix4f*>(transform))) : m_projection);
			}

			RML::LayerHandle PushLayer() override
			{
				++m_layer;

				if (m_layer >= m_layers.size())
				{
					auto pool = RHITexturePool::global_instance();
					m_layers.push_back(pool->acquire(RHISurfaceFormat::RGBA8, m_size, RHITextureFlags::RenderTarget));
				}

				EndRendering();

				m_context->push_debug_stage("Layer Rendering");
				return m_layer;
			}

			void CompositeLayers(RML::LayerHandle source, RML::LayerHandle destination, RML::BlendMode blend_mode,
			                     RML::Span<const RML::CompiledFilterHandle> filters) override
			{
				(void) blend_mode;
				(void) filters;

				if (source >= m_layers.size() || destination >= m_layers.size())
					return;

				EndRendering();

				m_context->barrier(m_layers[destination], RHIAccess::TransferDst);
				m_context->barrier(m_layers[source], RHIAccess::TransferSrc);
				m_context->copy(m_layers[destination], RHITextureRegion(m_size), m_layers[source], RHITextureRegion(m_size));
				m_context->barrier(m_layers[destination], RHIAccess::RTV);
				m_context->barrier(m_layers[source], RHIAccess::RTV);
			}

			void PopLayer() override
			{
				trinex_assert(m_layer > 0);
				EndRendering();

				--m_layer;
				m_context->pop_debug_stage();
			}

			RML::TextureHandle SaveLayerAsTexture() override
			{
				const Vector2u offset = Vector2u(m_scissor.pos * Vector2f(m_size) + Vector2f(0.5f, 0.5f));
				const Vector2u size   = Vector2u(m_scissor.size * Vector2f(m_size) + Vector2f(0.5f, 0.5f));

				RMLTextureCopy* texture = trx_new RMLTextureCopy(size, RHITextureFlags::ShaderResource);

				EndRendering();
				m_context->barrier(texture->texture(), RHIAccess::TransferDst);
				m_context->barrier(m_layers[m_layer], RHIAccess::TransferSrc);

				m_context->copy(texture->texture(), RHITextureRegion(size), m_layers[m_layer], RHITextureRegion(size, offset));

				m_context->barrier(texture->texture(), RHIAccess::SRVGraphics);
				m_context->barrier(m_layers[m_layer], RHIAccess::RTV);
				return texture->handle();
			}

			RML::CompiledFilterHandle SaveLayerAsMaskImage() override
			{
				trinex_unreachable();
				return 0;
			}

			RML::CompiledFilterHandle CompileFilter(const RML::String& name, const RML::Dictionary& parameters) override
			{
				return 0;
			}

			void ReleaseFilter(RML::CompiledFilterHandle filter) override {}

			RML::CompiledShaderHandle CompileShader(const RML::String& name, const RML::Dictionary& parameters) override
			{
				return 0;
			}

			void RenderShader(RML::CompiledShaderHandle shader, RML::CompiledGeometryHandle geometry, RML::Vector2f translation,
			                  RML::TextureHandle texture) override
			{}

			void ReleaseShader(RML::CompiledShaderHandle shader) override {}
		};

		class RMLSystemInterface : public RML::SystemInterface
		{
		public:
			RMLSystemInterface()           = default;
			~RMLSystemInterface() override = default;

			double GetElapsedTime() override { return engine_instance->time_seconds(); }

			void JoinPath(RML::String& translated_path, const RML::String& document_path, const RML::String& path) override
			{
				if (path.empty())
					return RML::SystemInterface::JoinPath(translated_path, document_path, path);

				for (char marker : {'@', '#', '$'})
				{
					if (marker == path[0])
					{
						translated_path = path;
						return;
					}
				}

				translated_path = (Path(Path(document_path.c_str()).base_path()) / Path(path.c_str())).str();
			}
		};

		class RMLFileInterface : public RML::FileInterface
		{
		public:
			RMLFileInterface()           = default;
			~RMLFileInterface() override = default;

			RML::FileHandle Open(const RML::String& path) override
			{
				auto file = rootfs()->open(path.c_str(), FileOpenMode::Read);
				return reinterpret_cast<RML::FileHandle>(file);
			}

			void Close(RML::FileHandle file) override
			{
				if (file)
				{
					rootfs()->close(reinterpret_cast<VFS::File*>(file));
				}
			}

			size_t Read(void* buffer, size_t size, RML::FileHandle file) override
			{
				if (file)
				{
					auto handle = reinterpret_cast<VFS::File*>(file);
					return handle->read(buffer, size);
				}
				return 0;
			}

			bool Seek(RML::FileHandle file, long offset, int origin) override
			{
				if (file)
				{
					auto handle = reinterpret_cast<VFS::File*>(file);

					FileSeekDir dir;

					if (origin == SEEK_SET)
						dir = FileSeekDir::Begin;
					else if (origin == SEEK_CUR)
						dir = FileSeekDir::Current;
					else
						dir = FileSeekDir::End;

					handle->rseek(offset, dir);
				}
				return 0;
			}

			size_t Tell(RML::FileHandle file) override
			{
				if (file)
				{
					return reinterpret_cast<VFS::File*>(file)->rpos();
				}
				return 0;
			}
		};

		class RMLClientPlugin final : public RML::Plugin
		{
		public:
			void OnDocumentLoad(RML::ElementDocument* document)
			{
				if (auto* client = RMLClient::from(document))
				{
					client->on_document_load(document);
				}
			}

			void OnDocumentUnload(RML::ElementDocument* document)
			{
				if (auto* client = RMLClient::from(document))
				{
					client->on_document_unload(document);
				}
			}

			static RMLClientPlugin* instance()
			{
				static RMLClientPlugin plugin;
				return &plugin;
			}
		};
	}// namespace

	trinex_on_pre_init({.name = "RML"})
	{
		RML::SetFileInterface(trx_new RMLFileInterface());
		RML::SetSystemInterface(trx_new RMLSystemInterface());
		RML::SetRenderInterface(trx_new RMLRenderInterface());
		RML::Initialise();
		RML::RegisterPlugin(RMLClientPlugin::instance());

		for (auto& path : VFS::RecursiveDirectoryIterator("[fonts]:/TrinexEditor/"))
		{
			auto extension = path.extension();

			static const StringView supported[] = {
			        ".ttf",
			        ".otf",
			};

			for (auto& ext : supported)
			{
				if (ext == extension)
				{
					trinex_verify(RML::LoadFontFace(path.str()));
					break;
				}
			}
		}
	}

	trinex_on_shutdown()
	{
		RML::UnregisterPlugin(RMLClientPlugin::instance());
		RML::Shutdown();

		trx_delete RML::GetFileInterface();
		trx_delete RML::GetSystemInterface();
		trx_delete RML::GetRenderInterface();

		RML::SetFileInterface(nullptr);
		RML::SetSystemInterface(nullptr);
		RML::SetRenderInterface(nullptr);
	}

	void RMLEngine::begin_rendering()
	{
		static_cast<RMLRenderInterface*>(RML::GetRenderInterface())->BeginRendering();
	}

	bool RMLEngine::is_rendering()
	{
		return static_cast<RMLRenderInterface*>(RML::GetRenderInterface())->IsRendering();
	}

	Vector2u RMLEngine::render_size()
	{
		return static_cast<RMLRenderInterface*>(RML::GetRenderInterface())->GetRenderSize();
	}

	RHITexture* RMLEngine::render_target()
	{
		return static_cast<RMLRenderInterface*>(RML::GetRenderInterface())->GetRenderTarget();
	}

	RHIContext* RMLEngine::render_context()
	{
		return static_cast<RMLRenderInterface*>(RML::GetRenderInterface())->GetContext();
	}

	void RMLEngine::end_rendering()
	{
		static_cast<RMLRenderInterface*>(RML::GetRenderInterface())->EndRendering();
	}

	void RMLEngine::render_texture(RHITexture* texture, const Vector2f& position, const Vector2f& size)
	{
		if (texture == nullptr)
			return;

		static_cast<RMLRenderInterface*>(RML::GetRenderInterface())->RenderTexture(texture, position, size);
	}

	trinex_implement_class(Trinex::UI::RMLClient, 0) {}

	RMLClient* RMLClient::from(RML::Element* element)
	{
		if (element == nullptr)
			return nullptr;

		RML::Context* context = element->GetContext();

		if (context == nullptr)
			return nullptr;

		RML::Element* root = context->GetRootElement();

		if (root == nullptr)
			return nullptr;

		return static_cast<RMLClient*>(root->GetAttribute<void*>("client", nullptr));
	}

	RMLClient& RMLClient::attach(class RenderViewport* viewport)
	{
		Super::attach(viewport);
		m_viewport = viewport;

		Vector2u size = viewport->size();
		m_context     = RML::CreateContext("main", RML::Vector2i(size.x, size.y));

		if (auto root = m_context->GetRootElement())
		{
			root->SetAttribute<void*>("client", this);
		}

		if (m_context)
		{
			if (!rml_debugger_initialized())
			{
				rml_debugger_initialized() = RML::Debugger::Initialise(m_context);
				RML::Debugger::SetVisible(false);
			}
			else
			{
				RML::Debugger::SetContext(m_context);
			}
		}

		return *this;
	}

	RMLClient& RMLClient::deattach(class RenderViewport* viewport)
	{
		Super::deattach(viewport);

		if (m_context)
		{
			RML::RemoveContext(m_context->GetName());
			m_context = nullptr;
		}

		m_viewport = nullptr;
		return *this;
	}

	RMLClient& RMLClient::update(class RenderViewport* viewport, float dt)
	{
		Super::update(viewport, dt);

		(void) dt;

		if (m_context)
		{
			{
				trinex_profile_cpu_n("RML Controllers Update");

				for (auto& [element, controller] : m_controllers)
				{
					if (element && controller)
					{
						controller->update(element);
					}
				}
			}

			Vector2u size = viewport->size();

			{
				trinex_profile_cpu_n("RML Context Update");
				m_context->SetDimensions(RML::Vector2i(size.x, size.y));
				m_context->Update();
			}

			{
				trinex_profile_cpu_n("RML Context Render");
				RMLRenderInterface* renderer = static_cast<RMLRenderInterface*>(RML::GetRenderInterface());
				renderer->BeginFrame(viewport->swapchain());
				m_context->Render();
				renderer->EndFrame(viewport->swapchain());
			}
		}

		return *this;
	}

	EventDispatchResult RMLClient::on_quit(RoutedEvent& event)
	{
		(void) event;

		EventDispatchResult result;
		result.mark_handled();
		return result;
	}

	EventDispatchResult RMLClient::on_window_event(WindowEvent& event)
	{
		EventDispatchResult result;

		if (!m_context)
			return result;

		switch (event.kind)
		{
			case WindowEventKind::Resized:
			{
				m_context->SetDimensions(RML::Vector2i(static_cast<int>(event.size.x), static_cast<int>(event.size.y)));
				result.mark_handled();
				return result;
			}

			case WindowEventKind::FocusLost:
			{
				const bool not_interacting = m_context->ProcessMouseLeave();
				result.mark_handled();
				if (!not_interacting)
					result.mark_consumed();
				return result;
			}

			case WindowEventKind::FocusGained:
			case WindowEventKind::Shown:
			case WindowEventKind::Hidden:
			case WindowEventKind::Moved:
			case WindowEventKind::CloseRequested: result.mark_handled(); return result;

			default: break;
		}

		return result;
	}

	EventDispatchResult RMLClient::on_key_event(KeyEvent& event)
	{
		EventDispatchResult result;

		if (!m_context)
			return result;

		const RML::Input::KeyIdentifier key_identifier = map_key_identifier(static_cast<KeyCode::Enum>(event.key_code));
		if (key_identifier == RML::Input::KI_UNKNOWN)
			return result;

		const int modifiers = key_modifier_state(event.header.source_id, event.header.window_id);

		if (event.kind == KeyEventKind::Pressed && key_identifier == RML::Input::KI_F12 && rml_debugger_initialized())
		{
			RML::Debugger::SetContext(m_context);
			RML::Debugger::SetVisible(!RML::Debugger::IsVisible());
			result.mark_handled();
			result.mark_consumed();
			return result;
		}

		const bool not_consumed = (event.kind == KeyEventKind::Released) ? m_context->ProcessKeyUp(key_identifier, modifiers)
		                                                                 : m_context->ProcessKeyDown(key_identifier, modifiers);

		result.mark_handled();
		if (!not_consumed)
			result.mark_consumed();

		return result;
	}

	EventDispatchResult RMLClient::on_text_input_event(TextInputEvent& event)
	{
		EventDispatchResult result;

		if (!m_context || event.codepoint == U'\0')
			return result;

		const bool not_consumed = m_context->ProcessTextInput(event.codepoint);
		result.mark_handled();
		if (!not_consumed)
			result.mark_consumed();

		return result;
	}

	EventDispatchResult RMLClient::on_pointer_event(PointerEvent& event)
	{
		EventDispatchResult result;

		if (!m_context)
			return result;

		const Identifier window_id =
		        (event.header.window_id != 0 ? event.header.window_id
		                                     : (m_viewport && m_viewport->window() ? m_viewport->window()->id() : 0));
		const int modifiers = key_modifier_state(event.header.source_id, window_id);

		switch (event.kind)
		{
			case PointerEventKind::Moved:
			case PointerEventKind::Entered:
			{
				const int x = static_cast<int>(event.screen_position.x);
				const int y =
				        static_cast<int>((m_viewport ? static_cast<int>(m_viewport->size().y) : 0) - event.screen_position.y);
				const bool not_consumed = m_context->ProcessMouseMove(x, y, modifiers);
				result.mark_handled();
				if (!not_consumed)
					result.mark_consumed();
				return result;
			}

			case PointerEventKind::ButtonPressed:
			{
				const int button_index = mouse_button_index(static_cast<MouseButton::Enum>(event.button));
				if (button_index < 0)
					return result;

				const bool not_consumed = m_context->ProcessMouseButtonDown(button_index, modifiers);
				result.mark_handled();
				if (!not_consumed)
					result.mark_consumed();
				return result;
			}

			case PointerEventKind::ButtonReleased:
			{
				const int button_index = mouse_button_index(static_cast<MouseButton::Enum>(event.button));
				if (button_index < 0)
					return result;

				const bool not_consumed = m_context->ProcessMouseButtonUp(button_index, modifiers);
				result.mark_handled();
				if (!not_consumed)
					result.mark_consumed();
				return result;
			}

			case PointerEventKind::Wheel:
			{
				const bool not_consumed =
				        m_context->ProcessMouseWheel(RML::Vector2f(event.wheel_delta.x, -event.wheel_delta.y), modifiers);
				result.mark_handled();
				if (!not_consumed)
					result.mark_consumed();
				return result;
			}

			case PointerEventKind::Left:
			{
				const bool not_consumed = m_context->ProcessMouseLeave();
				result.mark_handled();
				if (!not_consumed)
					result.mark_consumed();
				return result;
			}

			default: break;
		}

		return result;
	}

	RMLClient& RMLClient::on_document_load(RML::ElementDocument* document)
	{
		if (document)
		{
			collect_controllers(m_controllers, document, this);
		}

		return *this;
	}

	RMLClient& RMLClient::on_document_unload(RML::ElementDocument* document)
	{
		if (document == nullptr)
			return *this;

		for (auto it = m_controllers.begin(); it != m_controllers.end();)
		{
			RML::Element* element      = it->first;
			RMLController* controller  = it->second;
			RML::ElementDocument* root = element ? element->GetOwnerDocument() : nullptr;

			if (root == document)
			{
				if (controller)
				{
					controller->deattach(element);
					controller->owner(nullptr);
				}

				it = m_controllers.erase(it);
			}
			else
			{
				++it;
			}
		}

		return *this;
	}

	trinex_implement_class(Trinex::UI::RMLController, 0) {}

	RMLController& RMLController::attach(RML::Element* element)
	{
		return *this;
	}

	RMLController& RMLController::update(RML::Element* element)
	{
		return *this;
	}

	RMLController& RMLController::deattach(RML::Element* element)
	{
		return *this;
	}
}// namespace Trinex::UI
