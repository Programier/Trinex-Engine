#pragma once


namespace Editor
{
    class Panel
    {       
    protected:
        bool cursor_on_panel() const;
        Set<Panel*> _M_windows;


    public:
        void set_panel(Panel* panel);
        void remove_panel(Panel* panel);

        virtual void render() = 0;
        virtual void proccess_commands();
        virtual ~Panel();
    };
}// namespace Editor
