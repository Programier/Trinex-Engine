#pragma once
#include <Core/object.hpp>

namespace Engine
{
    class TabItem : public Object
    {
    protected:
        String _M_name;
        bool _M_is_open = true;

        static Set<String> _M_names;
    public:
        TabItem();
        void update();
        virtual void render() = 0;

        inline const String& item_name() const
        {
            return _M_name;
        }

        static TabItem* current();
        TabItem& make_current();
        void close_item();
        bool is_open();

        virtual ~TabItem();
    };
}
