#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/etl/array.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/file_manager.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/splash_config.hpp>
#include <Engine/splash_screen.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_2D.hpp>
#include <Platform/platform.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	struct TextInfo {
		String text;
		// ImFont* font;
		// ImVec2 text_size;
		int_t font_size;
		bool need_update_text_size = true;
	};

	struct SplashData {
		SplashConfig config;

		Atomic<bool> is_active = false;
		Thread* thread         = nullptr;
		Thread* exec_thread    = nullptr;
		Window* window         = nullptr;
		Texture2D* texture     = nullptr;
		Path font_path;

		Array<TextInfo, static_cast<size_t>(SplashTextType::Count)> text_infos;


		SplashData()
		{
			text_info_of(SplashTextType::StartupProgress).font_size = config.startup_text_size;
			text_info_of(SplashTextType::VersionInfo).font_size     = config.version_text_size;
			text_info_of(SplashTextType::CopyrightInfo).font_size   = config.copyright_text_size;
			text_info_of(SplashTextType::GameName).font_size        = config.game_name_text_size;
		}

		TextInfo& text_info_of(size_t index)
		{
			return text_infos[index];
		}

		TextInfo& text_info_of(SplashTextType type)
		{
			return text_info_of(static_cast<size_t>(type));
		}
	};

	static SplashData* m_splash_data = nullptr;

	// class SplashClient : public ViewportClient
	// {
	// 	declare_class(SplashClient, ViewportClient);

	// public:
	// 	ViewportClient& on_bind_viewport(class RenderViewport* viewport) override
	// 	{
	// 		Window* window = viewport->window();

	// 		window->imgui_initialize([](ImGuiContext* ctx) {
	// 			auto& style            = ImGui::GetStyle();
	// 			style.WindowPadding    = {0, 0};
	// 			style.WindowBorderSize = 0.f;

	// 			FileReader reader(m_splash_data->config.font_path);

	// 			if (reader.is_open())
	// 			{
	// 				Buffer buffer = reader.read_buffer();
	// 				ImFontConfig config;
	// 				config.FontDataOwnedByAtlas = false;


	// 				size_t count = static_cast<size_t>(SplashTextType::Count);

	// 				for (size_t i = 0; i < count; i++)
	// 				{
	// 					auto& info = m_splash_data->text_info_of(i);
	// 					info.font =
	// 					        ImGui::GetIO().Fonts->AddFontFromMemoryTTF(buffer.data(), buffer.size(), info.font_size, &config,
	// 					                                                   ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	// 				}
	// 			}
	// 		});

	// 		return *this;
	// 	}

	// 	ViewportClient& render(class RenderViewport* viewport) override
	// 	{
	// 		viewport->rhi_bind();
	// 		viewport->window()->imgui_window()->rhi_render();
	// 		return *this;
	// 	}

	// 	static void render_text(SplashTextType type)
	// 	{
	// 		auto& info = m_splash_data->text_info_of(type);
	// 		ImGui::PushFont(info.font);

	// 		if (info.need_update_text_size)
	// 		{
	// 			info.text_size = ImGui::CalcTextSize(info.text.c_str());
	// 		}

	// 		auto list = ImGui::GetWindowDrawList();
	// 		auto pos  = ImGui::GetCursorScreenPos();

	// 		list->AddRectFilled(pos, pos + info.text_size, ImGui::GetColorU32({0.25, 0.25, 0.25, 0.5}));

	// 		ImGui::TextUnformatted(info.text.c_str());
	// 		ImGui::PopFont();
	// 	}

	// 	ViewportClient& update(class RenderViewport* viewport, float dt) override
	// 	{
	// 		auto imgui_window = viewport->window()->imgui_window();
	// 		imgui_window->new_frame();

	// 		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
	// 		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);
	// 		ImGui::Begin("SplashScreen", nullptr,
	// 		             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);

	// 		static constexpr float padding = 10.f;
	// 		auto pos                       = ImGui::GetCursorPos() + ImVec2(padding, padding);
	// 		auto size                      = ImGui::GetContentRegionAvail();
	// 		ImGui::Image(m_splash_data->texture, size);

	// 		size -= ImVec2(2 * padding, 2 * padding);

	// 		ImGui::SetCursorPos(pos);
	// 		// Left side
	// 		ImGui::BeginVertical(0, size, 0.0);

	// 		ImGui::Spring(1);
	// 		render_text(SplashTextType::GameName);
	// 		render_text(SplashTextType::VersionInfo);
	// 		render_text(SplashTextType::StartupProgress);
	// 		ImGui::Spring(0);
	// 		ImGui::EndVertical();

	// 		ImGui::SetCursorPos(pos);

	// 		// Right side
	// 		ImGui::BeginVertical(1, size, 1.f);
	// 		ImGui::Spring(1);
	// 		render_text(SplashTextType::CopyrightInfo);
	// 		ImGui::Spring(0);
	// 		ImGui::EndVertical();

	// 		ImGui::End();

	// 		imgui_window->end_frame();
	// 		return *this;
	// 	}
	// };

	// implement_engine_class_default_init(SplashClient, 0);

	struct SplashUpdate : public Task<SplashUpdate> {
		void execute() override
		{
			RenderViewport* viewport = m_splash_data->window->render_viewport();
			viewport->update(0.033f);
			viewport->render();
			render_thread()->call([]() { rhi->submit(); });
		}
	};

	static void splash_main()
	{
		while (m_splash_data->is_active)
		{
			m_splash_data->exec_thread->create_task<SplashUpdate>();
			ThisThread::sleep_for(0.033f);
		}

		m_splash_data->exec_thread->wait();
	}

	ENGINE_EXPORT void show_splash_screen()
	{
		return;
		if (m_splash_data)
			return;

		m_splash_data = new SplashData();
		Image image(m_splash_data->config.image_path);

		if (image.empty())
		{
			hide_splash_screen();
			return;
		}

		m_splash_data->texture = Object::new_instance<EngineResource<Texture2D>>();
		m_splash_data->texture->init(image);

		WindowConfig window_config;

		window_config.attributes = {WindowAttribute::BorderLess};
		window_config.title      = "Splash Screen";
		window_config.client     = "";
		window_config.size       = image.size();
		window_config.size =
		        (window_config.size / window_config.size.x) * static_cast<float>(Platform::monitor_info().size.x) / 3.f;
		window_config.position = {-1, -1};
		window_config.vsync    = true;

		m_splash_data->window = WindowManager::instance()->create_window(window_config);
		//m_splash_data->window->render_viewport()->client(Object::new_instance<SplashClient>());

		m_splash_data->thread      = new Thread();
		m_splash_data->exec_thread = new Thread();
		m_splash_data->is_active   = true;

		struct SplashMain : public Task<SplashMain> {
			void execute()
			{
				splash_main();
			}
		};

		m_splash_data->thread->create_task<SplashMain>();
	}

	ENGINE_EXPORT void splash_screen_text(SplashTextType type, const StringView& text)
	{
		return;
		class UpdateText : public Task<UpdateText>
		{
			SplashTextType m_type;
			String m_text;

		public:
			UpdateText(SplashTextType type, const StringView& text) : m_type(type), m_text(text)
			{}

			void execute() override
			{
				auto& info                 = m_splash_data->text_info_of(m_type);
				info.text                  = m_text;
				info.need_update_text_size = true;
			}
		};

		if (m_splash_data)
		{
			m_splash_data->exec_thread->create_task<UpdateText>(type, text);
		}
	}


	ENGINE_EXPORT void stop_thread(Thread*& thread)
	{
		if (thread)
		{
			thread->wait();
			delete thread;
			thread = nullptr;
		}
	}

	ENGINE_EXPORT void hide_splash_screen()
	{
		return;
		if (m_splash_data == nullptr)
			return;
		m_splash_data->is_active = false;
		stop_thread(m_splash_data->thread);
		stop_thread(m_splash_data->exec_thread);

		if (m_splash_data->window)
		{
			WindowManager::instance()->destroy_window(m_splash_data->window);
			m_splash_data->window = nullptr;
		}

		if (m_splash_data->texture)
		{
			GarbageCollector::destroy(m_splash_data->texture);
		}

		delete m_splash_data;
		m_splash_data = nullptr;
	}
}// namespace Engine
