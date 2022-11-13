
#include <Core/logger.hpp>
#include <Core/string_convert.hpp>
#include <Graphics/drawable_object.hpp>
#include <Graphics/scene.hpp>
#include <TemplateFunctional/instanceof.hpp>

namespace Engine
{

    DrawableObject::DrawableObject() : _M_aabb({{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}})
    {
        // Init callbacks
        _M_on_translate.insert(DrawableObject::on_translate);
        _M_on_rotate.insert(DrawableObject::on_rotate);
        _M_on_scale.insert(DrawableObject::on_scale);
    }

    void DrawableObject::on_translate(Translate* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);

        if (object)
            on_changed_spatial_parameters(object);
    }

    void DrawableObject::on_rotate(Rotate* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);
        if (object)
            on_changed_spatial_parameters(object);
    }

    void DrawableObject::on_scale(Scale* self)
    {
        DrawableObject* object = dynamic_cast<DrawableObject*>(self);
        if (object)
            on_changed_spatial_parameters(object);
    }

    void DrawableObject::on_changed_spatial_parameters(DrawableObject* self)
    {}

    const AABB_3D& DrawableObject::aabb() const
    {
        return _M_aabb.aabb();
    }

    void DrawableObject::recalculate_aabb()
    {
        struct Node {
            DrawableObject* obj;
            glm::mat4 matrix;

            Node(DrawableObject* _obj, const glm::mat4& mat) : obj(_obj), matrix(mat)
            {}
        };

        std::list<Node> _M_stack = {Node(this, this->model())};
    }

    DrawableObject& DrawableObject::aabb(const AABB_3D& aabb)
    {
        if (_M_sub_objects.size() != 0)
            return *this;

        _M_aabb = BoxHB(aabb);
        _M_original_aabb = BoxHB(aabb);
        return *this;
    }

    DrawableObject& DrawableObject::push_object(DrawableObject* object)
    {
        if (!object || is_instance_of<Scene>(object) || root()->contains_item(object))
        {
            return *this;
        }

        object->recalculate_aabb();
        _M_sub_objects.insert(object);
        object->_M_parent = this;


        Scene* _M_scene = scene();


        return *this;
    }

    DrawableObject& DrawableObject::remove_object(DrawableObject* object)
    {
        if (object && object->_M_parent == this)
        {
            _M_sub_objects.erase(object);
            object->_M_parent = nullptr;
        }
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

    void DrawableObject::render()
    {
        using Node = std::pair<DrawableObject*, glm::mat4>;
        std::list<Node> _M_stack = {{this, Constants::identity_matrix}};

        Scene* _scene = this->scene();


        if (!_scene)
        {
            throw std::runtime_error("No scene found!");
        }

        Scene* active = _scene->get_active_scene();

        if (!_scene->contains_item(active))
        {
            throw std::runtime_error("No scene found!");
        }

        if (active->active_camera().camera == nullptr)
        {
            throw std::runtime_error("No active camera found found!");
        }

        Frustum frustum(*active->active_camera().camera);

#ifdef ENGINE_DEBUG
        std::size_t cpu_processed = 0;
        std::size_t gpu_processed = 0;
#endif

        while (!_M_stack.empty())
        {
            Node node = _M_stack.back();
            _M_stack.pop_back();
#ifdef ENGINE_DEBUG
            cpu_processed += cast(int, node.first->test);
#endif

            auto matrix = node.second * node.first->model();
            if ((node.first->_M_aabb.is_in_frustum(frustum, matrix)) || !node.first->test)
            {
                node.first->render_layer(node.second);
#ifdef ENGINE_DEBUG
                gpu_processed += cast(int, node.first->test);
#endif

                for (auto& ell : node.first->_M_sub_objects)
                {
                    _M_stack.push_back({ell, node.second * node.first->_M_model.get()});
                }
            }
        }

#ifdef ENGINE_DEBUG
        if (Event::frame_number() % 60 == 0)
            logger->log("Total processed on cpu: %zu, proccessed on gpu: %zu\n", cpu_processed, gpu_processed);
#endif
    }

    void DrawableObject::render_layer(const glm::mat4& prev_model)
    {
        // empty method
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
