#pragma once

#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Graphics/basic_object.hpp>
#include <Graphics/drawable_loader.hpp>
#include <Graphics/hitbox.hpp>
#include <Graphics/octree.hpp>
#include <unordered_set>
#include <vector>

namespace Engine
{
    class Scene;
    class DrawableObject;
    void empty_drawable_callback_handler(const DrawableObject*, const glm::mat4&);
    using on_render_layer_func = void (*)(const DrawableObject*, const glm::mat4&);

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
        void on_changed_spatial_parameters();
        void on_before_changed_spatial_parameters();

        static void on_set_model(ModelMatrix* self);
        static void on_before_set_model(ModelMatrix* self);
        static void on_scale(Scale* self);
        static void on_translate(Translate* self);
        static void on_rotate(Rotate* self);
        static void on_before_scale(Scale* self);
        static void on_before_translate(Translate* self);
        static void on_before_rotate(Rotate* self);

        void private_remove_object(DrawableObject* object);

        mutable glm::mat4 _M_cached_matrix = Constants::identity_matrix;

        void unlink_object_from_head();
        void link_object_to_head();
        void repush_object();

        void update_aabb();

    protected:
        DrawableObject* _M_parent = nullptr;
        BoxHB _M_original_aabb;
        BoxHB _M_aabb;

        bool _M_visible = true;
        std::wstring _M_name;
        std::size_t _M_sub_objects_count = 0;

        Octree<DrawableObject*> _M_octree;
        std::unordered_set<DrawableObject*> _M_sub_objects;

        DrawableObject();

        const glm::mat4& get_cached_matrix() const;

    public:
        glm::mat4 global_model();
        DrawableObject& push_object(DrawableObject* object);
        DrawableObject& remove_object(DrawableObject* object);
        const std::unordered_set<DrawableObject*> sub_objects() const;
        bool visible() const;
        DrawableObject& visible(bool status);
        const BoxHB& aabb() const;
        const BoxHB original_aabb() const;
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
        const Octree<DrawableObject*>& octree() const;


        virtual void render_layer(const glm::mat4& prev_model = Constants::identity_matrix,
                                  on_render_layer_func = empty_drawable_callback_handler) const;
        virtual void render(on_render_layer_func = empty_drawable_callback_handler) const;
        virtual bool is_empty_layer() const = 0;
        virtual ~DrawableObject();
    };

}// namespace Engine
