
#include <Core/logger.hpp>
#include <Core/string_convert.hpp>
#include <Graphics/drawable_object.hpp>
#include <Graphics/scene.hpp>
#include <TemplateFunctional/instanceof.hpp>

namespace Engine
{

    DrawableObject::DrawableObject() : _M_aabb({0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}), _M_octree(0.01f)
    {
        // Init callbacks
        _M_on_before_set_model.insert(DrawableObject::on_before_set_model);
        _M_on_set_model.insert(DrawableObject::on_set_model);

        _M_on_translate.insert(DrawableObject::on_translate);
        _M_on_rotate.insert(DrawableObject::on_rotate);
        _M_on_scale.insert(DrawableObject::on_scale);

        _M_on_before_translate.insert(DrawableObject::on_before_translate);
        _M_on_before_rotate.insert(DrawableObject::on_before_rotate);
        _M_on_before_scale.insert(DrawableObject::on_before_scale);
    }

    void DrawableObject::on_set_model(ModelMatrix* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);

        if (object)
            object->on_changed_spatial_parameters();
    }

    void DrawableObject::on_before_set_model(ModelMatrix* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);

        if (object)
            object->on_before_changed_spatial_parameters();
    }

    void DrawableObject::on_before_translate(Translate* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);

        if (object)
            object->on_before_changed_spatial_parameters();
    }

    void DrawableObject::on_before_rotate(Rotate* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);
        if (object)
            object->on_before_changed_spatial_parameters();
    }

    void DrawableObject::on_before_scale(Scale* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);
        if (object)
            object->on_before_changed_spatial_parameters();
    }

    void DrawableObject::on_translate(Translate* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);

        if (object)
            object->on_changed_spatial_parameters();
    }

    void DrawableObject::on_rotate(Rotate* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);
        if (object)
            object->on_changed_spatial_parameters();
    }

    void DrawableObject::on_scale(Scale* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);
        if (object)
            object->on_changed_spatial_parameters();
    }

    void DrawableObject::on_before_changed_spatial_parameters()
    {
        unlink_object_from_head();
    }

    void DrawableObject::on_changed_spatial_parameters()
    {
        _M_cached_matrix = model();
        _M_octree.clear();
        _M_aabb = _M_original_aabb.apply_model(_M_cached_matrix);

        for (auto& object : _M_sub_objects)
        {
            auto box = object->_M_aabb.apply_model(_M_cached_matrix);
            _M_octree.push(object, box);
        }

        update_aabb();

        link_object_to_head();
    }

    const BoxHB& DrawableObject::aabb() const
    {
        return _M_aabb;
    }

    const BoxHB DrawableObject::original_aabb() const
    {
        return _M_original_aabb;
    }

    const glm::mat4& DrawableObject::get_cached_matrix() const
    {
        return _M_cached_matrix;
    }

    glm::mat4 DrawableObject::global_model()
    {
        // Generate stack
        std::list<DrawableObject*> _M_stack = {this};
        while (!_M_stack.back()->_M_parent) _M_stack.push_back(_M_stack.back()->_M_parent);

        // Calculate result matrix
        glm::mat4 _M_global_matrix = Constants::identity_matrix;

        while (!_M_stack.empty())
        {
            _M_global_matrix = _M_stack.front()->model() * _M_global_matrix;
            _M_stack.pop_front();
        }

        return _M_global_matrix;
    }

    DrawableObject& DrawableObject::aabb(const AABB_3D& aabb)
    {
        if (_M_sub_objects.size() != 0)
            return *this;

        _M_original_aabb = BoxHB(aabb);
        _M_aabb = _M_original_aabb.apply_model(model());

        return *this;
    }

    void DrawableObject::unlink_object_from_head()
    {
        DrawableObject* layer = this;
        while (layer->_M_parent)
        {
            layer->_M_parent->private_remove_object(layer);
            layer = layer->_M_parent;
        }
    }

    void DrawableObject::update_aabb()
    {
        _M_aabb = _M_original_aabb.apply_model(_M_cached_matrix);
        auto head = _M_octree.head();
        if (head)
            _M_aabb = _M_aabb.half_size().x == 0.f ? head->box() : _M_aabb.max_box(head->box());
    }

    void DrawableObject::link_object_to_head()
    {
        DrawableObject* layer = this->_M_parent;
        DrawableObject* object = this;
        while (layer)
        {
            auto box = object->_M_aabb.apply_model(layer->_M_cached_matrix);
            layer->_M_octree.push(object, box);
            layer->update_aabb();
            layer->_M_sub_objects.insert(object);
            object = layer;
            layer = layer->_M_parent;
        }
    }


    void DrawableObject::repush_object()
    {
        unlink_object_from_head();
        link_object_to_head();
    }

    DrawableObject& DrawableObject::push_object(DrawableObject* object)
    {
        if (!object || is_instance_of<Scene>(object) || root()->contains_item(object))
        {
            return *this;
        }

        if (object->_M_parent)
            object->_M_parent->private_remove_object(object);

        object->_M_parent = this;
        unlink_object_from_head();
        auto box = object->_M_aabb.apply_model(_M_cached_matrix);

        _M_octree.push(object, box);
        _M_sub_objects.insert(object);
        update_aabb();
        link_object_to_head();
        return *this;
    }

    void DrawableObject::private_remove_object(DrawableObject* object)
    {
        if (object && object->_M_parent == this)
        {
            _M_sub_objects.erase(object);
            auto octree_node = _M_octree.find(object->aabb().apply_model(_M_cached_matrix));
            if (!octree_node)
                return;
            octree_node->values.erase(object);

            if (!octree_node->values.empty())
                return;

            bool need_delete = true;
            for (byte i = 0; i < 8 && need_delete; i++)
            {
                if (octree_node->get(i))
                    need_delete = false;
            }

            if (need_delete)
                octree_node->destroy();
        }
    }

    DrawableObject& DrawableObject::remove_object(DrawableObject* object)
    {
        private_remove_object(object);
        object->_M_parent = nullptr;
        return *this;
    }

    const std::unordered_set<DrawableObject*> DrawableObject::sub_objects() const
    {
        return _M_sub_objects;
    }

    bool DrawableObject::visible() const
    {
        return _M_visible;
    }

    DrawableObject& DrawableObject::visible(bool status)
    {
        for (auto& obj : *this)
        {
            obj._M_visible = status;
        }
        return *this;
    }

    DrawableObject* DrawableObject::parent()
    {
        return _M_parent;
    }

    const DrawableObject* DrawableObject::parent() const
    {
        return _M_parent;
    }


    DrawableObject::basic_iterator DrawableObject::begin()
    {
        return basic_iterator(this);
    }

    DrawableObject::basic_iterator DrawableObject::end()
    {
        return basic_iterator(this, true);
    }

    DrawableObject::const_iterator DrawableObject::begin() const
    {
        return const_iterator(const_cast<DrawableObject*>(this));
    }

    DrawableObject::const_iterator DrawableObject::end() const
    {
        return const_iterator(const_cast<DrawableObject*>(this), true);
    }

    const std::wstring& DrawableObject::name() const
    {
        return _M_name;
    }

    DrawableObject& DrawableObject::name(const std::wstring& name)
    {
        _M_name = name;
        return *this;
    }


    void empty_drawable_callback_handler(const DrawableObject*, const glm::mat4&)
    {}

    void DrawableObject::render(void (*on_render_layer)(const DrawableObject*, const glm::mat4&)) const
    {
        if (!_M_visible)
            return;

        // Try to find scene
        const Scene* _M_scene = scene();
        if (!_M_scene || !_M_scene->_M_active_camera.camera)
            throw std::runtime_error("No scene or camera found!");

        Frustum frustum(*_M_scene->_M_active_camera.camera);

        // List of allocated objects
        std::list<glm::mat4*> _M_matrix_objects;

        // Create node for stack
        using OctreeNode = const Octree<DrawableObject*>::OctreeNode*;

        struct Node {
            OctreeNode _M_node;
            glm::mat4* _M_matrix;
            glm::mat4* _M_prev_matrix;

            Node(OctreeNode node = nullptr, glm::mat4* matrix = nullptr, glm::mat4* prev_matrix = nullptr)
                : _M_node(node), _M_matrix(matrix), _M_prev_matrix(prev_matrix)
            {}
        };

        render_layer(Constants::identity_matrix, on_render_layer);

        std::list<Node> _M_stack;

        // Init first node
        {
            auto octree_head = _M_octree.head();
            if (octree_head)
            {
                glm::mat4* prev_matrix = new glm::mat4(1.f);
                glm::mat4* current_matrix = new glm::mat4(_M_cached_matrix);
                _M_stack.emplace_back(octree_head, current_matrix, prev_matrix);

                _M_matrix_objects.push_back(prev_matrix);
                _M_matrix_objects.push_back(current_matrix);
            }
        }

        while (!_M_stack.empty())
        {
            Node node = _M_stack.back();
            _M_stack.pop_back();

            if (node._M_node->box().is_in_frustum(frustum, *node._M_prev_matrix))
            {
                for (auto& ell : node._M_node->values)
                {
                    if (!ell->_M_visible)
                        continue;

                    glm::mat4* global_ell_matrix = new glm::mat4((*node._M_matrix) * ell->_M_cached_matrix);
                    _M_matrix_objects.push_back(global_ell_matrix);

                    if (!ell->is_empty_layer() && ell->_M_original_aabb.is_in_frustum(frustum, *global_ell_matrix))
                        ell->render_layer(*node._M_matrix, on_render_layer);


                    auto octree_head = ell->_M_octree.head();
                    if (octree_head)
                    {
                        _M_stack.emplace_back(octree_head, global_ell_matrix, node._M_matrix);
                    }
                }

                for (byte i = 0; i < 8; i++)
                {
                    auto sub_node = node._M_node->get(i);
                    if (sub_node)
                    {
                        _M_stack.emplace_back(sub_node, node._M_matrix, node._M_prev_matrix);
                    }
                }
            }
        }

        // Free memory of matrix list
        for (auto& ell : _M_matrix_objects) delete ell;
    }


    void DrawableObject::render_layer(const glm::mat4& prev_model,
                                      void (*on_render_layer)(const DrawableObject*, const glm::mat4&)) const
    {
        // empty method
    }

    const Octree<DrawableObject*>& DrawableObject::octree() const
    {
        const Scene* _M_scene = scene();
        if (!_M_scene)
            throw std::runtime_error("No scene found!");
        return _M_scene->_M_octree;
    }

    DrawableObject& DrawableObject::load(const std::string& filename, const ObjectLoader::DrawableLoader& loader)
    {
        return push_object(loader.load(filename));
    }

    template<typename Type, typename Type2>
    Type* get_scene(Type2 object)
    {
        while (object && !is_instance_of<Type>(object))
        {
            object = object->parent();
        }

        return dynamic_cast<Type*>(object);
    }


    Scene* DrawableObject::scene()
    {
        return get_scene<Scene>(this);
    }

    const Scene* DrawableObject::scene() const
    {
        return get_scene<const Scene>(this);
    }


    bool DrawableObject::contains_item(const DrawableObject* object) const
    {
        for (const DrawableObject& item : *this)
        {
            if (&item == object)
                return true;
        }
        return false;
    }

    bool DrawableObject::contains_item(const DrawableObject& object) const
    {
        return contains_item(&object);
    }


    template<typename Type>
    Type get_root_of(Type address)
    {
        Type parent = address->parent();
        while (parent)
        {
            address = parent;
            parent = parent->parent();
        }

        return address;
    }

    DrawableObject* DrawableObject::root()
    {
        return get_root_of(this);
    }

    const DrawableObject* DrawableObject::root() const
    {
        return get_root_of(this);
    }

    DrawableObject::~DrawableObject()
    {
        for (auto& sub_obj : _M_sub_objects)
        {
            for (auto& obj : *sub_obj)
            {
                delete &obj;
            }
        }
    }
}// namespace Engine
