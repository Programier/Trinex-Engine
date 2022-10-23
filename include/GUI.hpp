#pragma once



namespace Engine
{
    class Application;

    namespace GUI
    {
        void init(Application *app);
        void render();
        void terminate();
        void init_logger();
    }
}
