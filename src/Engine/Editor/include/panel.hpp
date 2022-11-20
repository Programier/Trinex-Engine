#pragma once

namespace Editor
{
    class Panel
    {
    protected:
        bool cursor_on_panel() const;
    public:
        virtual void render() = 0;
        virtual ~Panel();
    };
}// namespace Editor
