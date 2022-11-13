#pragma once

#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/drawable_loader.hpp>
#include <unordered_set>
#include <vector>
#include <Graphics/hitbox.hpp>

namespace Engine
{
    class Scene;

    class ENGINE_EXPORT DrawableObject : virtual public BasicObject<Translate, Rotate, Scale>
    {

    public:
        // Iterators

        template<typename Access>
        CLASS Iterator
        {
        protected:
            Access* _M_current = nullptr;
            Access* _M_root = nullptr;
            std::vector<Access*> _M_stack;
            bool _M_is_end = false;

            Iterator(Access * root, bool is_end = false)
            {
                _M_stack.reserve(root->_M_sub_objects_count + 1);
                _M_current = _M_root = root;
                _M_is_end = is_end;
            }

        public:
#if __cplusplus >= 201703L
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = Access;
            using pointer = value_type*;
            using reference = value_type&;
#endif


            Iterator(const Iterator&) = default;
            Iterator(Iterator &&) = default;
            Iterator& operator=(const Iterator&) = default;
            Iterator& operator=(Iterator&&) = default;
            Iterator& operator++()
            {
                for (auto address : _M_current->_M_sub_objects)
                {
                    _M_stack.push_back(address);
                }

                if (!_M_stack.empty())
                {
                    _M_current = _M_stack.back();
                    _M_stack.pop_back();
                }
                else
                {
                    _M_current = _M_root;
                    _M_is_end = !_M_is_end;
                }

                return *this;
            }

            Iterator operator++(int)
            {
                auto prev = *this;
                ++(*this);
                return prev;
            }

            bool operator==(const Iterator& it) const
            {
                return _M_current == it._M_current && it._M_is_end == _M_is_end;
            }

            bool operator!=(const Iterator& it) const
            {
                return !(*this == it);
            }

            reference& operator*() const
            {
                return *_M_current;
            }

            pointer base() const
            {
                return _M_current;
            }

            friend class DrawableObject;
        };

        using basic_iterator = Iterator<DrawableObject>;
        using const_iterator = Iterator<const DrawableObject>;

    private:
        static void on_changed_spatial_parameters(DrawableObject* self);
        static void on_scale(Scale* self);
        static void on_translate(Translate* self);
        static void on_rotate(Rotate* self);

    protected:
        DrawableObject* _M_parent = nullptr;
        BoxHB _M_original_aabb;
        BoxHB _M_aabb;

        bool _M_visible = true;
        std::wstring _M_name;
        std::size_t _M_sub_objects_count = 0;

        std::unordered_set<DrawableObject*> _M_sub_objects;

        DrawableObject();

    public:
        bool test = false;
        void recalculate_aabb();
        DrawableObject& push_object(DrawableObject* object);
        DrawableObject& remove_object(DrawableObject* object);
        const std::unordered_set<DrawableObject*> sub_objects() const;
        bool visible() const;
        DrawableObject& visible(bool status);
        const AABB_3D& aabb() const;
        DrawableObject& aabb(const AABB_3D& aabb);
        DrawableObject* parent();
        const DrawableObject* parent() const;

        basic_iterator begin();
        basic_iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        bool contains_item(const DrawableObject* object) const;
        bool contains_item(const DrawableObject& object) const;

        const std::wstring& name() const;
        DrawableObject& name(const std::wstring& name);

        virtual DrawableObject* copy() const = 0;
        DrawableObject& load(const std::string& filename,
                             const ObjectLoader::DrawableLoader& loader = ObjectLoader::PolygonalMeshLoader());

        DrawableObject* root();
        const DrawableObject* root() const;

        Scene* scene();
        const Scene* scene() const;

        virtual void render_layer(const glm::mat4& prev_model = Constants::identity_matrix);
        void render();
        virtual ~DrawableObject();
    };

}// namespace Engine
