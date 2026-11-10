#include "PyFacade/DocumentFacade.h"

#include "PyFacade/SceneGateway.h"

#include "Engine2D/Core/SceneManager.h"
#include "Engine2D/Edit/SceneEditService.h"
#include "Engine2D/AprEntity/AprLine.h"
#include "Engine2D/AprEntity/AprPoint.h"
#include "Engine2D/AprEntity/AprCircle.h"

#include "Ut/Vec.h"

namespace PyFacade
{
    namespace
    {
        Ut::Vec2d toUtVec2(const Vec2& v)
        {
            return Ut::Vec2d(v.x, v.y);
        }

        // 与 SceneGateway 的同名转换函数区分：unity 合批后两个匿名命名空间会并入同一
        // 翻译单元，同名定义触发 C2084「已有主体」。
        Ut::BBox2d toUtBBox2Doc(const BBox2& box)
        {
            return Ut::BBox2d(Ut::Vec2d(box.minX, box.minY), Ut::Vec2d(box.maxX, box.maxY));
        }

        EntityRef makeRef(Eg::EntityId id)
        {
            return EntityRef{ id };
        }
    }  // namespace

    DocumentFacade::DocumentFacade()
        : m_scene(std::make_unique<Eg::SceneManager>())
    {
    }

    DocumentFacade::DocumentFacade(std::unique_ptr<Eg::SceneManager> scene, std::string path)
        : m_scene(std::move(scene))
        , m_path(std::move(path))
    {
    }

    DocumentFacade::~DocumentFacade() = default;

    std::shared_ptr<DocumentFacade> DocumentFacade::create()
    {
        return std::shared_ptr<DocumentFacade>(new DocumentFacade());
    }

    std::pair<std::shared_ptr<DocumentFacade>, std::string> DocumentFacade::open(const std::string& /*path*/)
    {
        return { nullptr, "Document.open is not implemented yet; use Document.create() for in-memory scenes" };
    }

    bool DocumentFacade::isValid() const
    {
        return static_cast<bool>(m_scene);
    }

    std::string DocumentFacade::path() const
    {
        return m_path;
    }

    std::string DocumentFacade::lastError() const
    {
        return m_lastError;
    }

    EntityRef DocumentFacade::addPoint(const Vec2& point)
    {
        if (!m_scene)
        {
            setError("Document is closed");
            return EntityRef{};
        }

        auto entity = std::make_unique<Eg::AprPoint>(toUtVec2(point));
        const Eg::EntityId id = entity->id;
        // SceneEditService 只持有裸指针，这里从 unique_ptr 取地址，不转移所有权
        SceneEditService directEdit(m_scene.get(), nullptr);
        if (!directEdit.insertEntityPreserveId(std::move(entity)))
        {
            setError("Failed to add point");
            return EntityRef{};
        }

        m_lastError.clear();
        return makeRef(id);
    }

    EntityRef DocumentFacade::addLine(const Vec2& start, const Vec2& end)
    {
        if (!m_scene)
        {
            setError("Document is closed");
            return EntityRef{};
        }

        const std::vector<Ut::Vec2d> points = { toUtVec2(start), toUtVec2(end) };
        auto entity = std::make_unique<Eg::AprLine>(points);
        const Eg::EntityId id = entity->id;
        // SceneEditService 只持有裸指针，这里从 unique_ptr 取地址，不转移所有权
        SceneEditService directEdit(m_scene.get(), nullptr);
        if (!directEdit.insertEntityPreserveId(std::move(entity)))
        {
            setError("Failed to add line");
            return EntityRef{};
        }

        m_lastError.clear();
        return makeRef(id);
    }

    EntityRef DocumentFacade::addCircle(const Vec2& center, double radius)
    {
        if (!m_scene)
        {
            setError("Document is closed");
            return EntityRef{};
        }

        if (radius <= 0.0)
        {
            setError("Circle radius must be positive");
            return EntityRef{};
        }

        auto entity = std::make_unique<Eg::AprCircle>();
        entity->basePoint = toUtVec2(center);
        entity->dRadius = radius;
        const Eg::EntityId id = entity->id;
        // SceneEditService 只持有裸指针，这里从 unique_ptr 取地址，不转移所有权
        SceneEditService directEdit(m_scene.get(), nullptr);
        if (!directEdit.insertEntityPreserveId(std::move(entity)))
        {
            setError("Failed to add circle");
            return EntityRef{};
        }

        m_lastError.clear();
        return makeRef(id);
    }

    bool DocumentFacade::removeEntity(EntityRef ref)
    {
        if (!m_scene || !ref.valid())
        {
            setError("Invalid entity reference");
            return false;
        }

        Eg::AprEntity* entity = m_scene->findAprEntityById(ref.id);
        if (!entity)
        {
            setError("Entity not found");
            return false;
        }

        // SceneEditService 只持有裸指针，这里从 unique_ptr 取地址，不转移所有权
        SceneEditService directEdit(m_scene.get(), nullptr);
        directEdit.deleteEntitiesDirect({ entity });
        m_lastError.clear();
        return true;
    }

    std::vector<EntityRef> DocumentFacade::queryBox(const BBox2& box, bool containedOnly) const
    {
        std::vector<EntityRef> refs;
        if (!m_scene || !box.isValid())
        {
            return refs;
        }

        const Eg::VecAprEntityPtr entities = m_scene->queryByBox(toUtBBox2Doc(box), containedOnly);
        refs.reserve(entities.size());
        for (Eg::AprEntity* entity : entities)
        {
            if (entity)
            {
                refs.push_back(makeRef(entity->id));
            }
        }
        return refs;
    }

    SceneSnapshot DocumentFacade::exportSnapshot() const
    {
        if (!m_scene)
        {
            return SceneSnapshot{};
        }
        return PyFacade::exportSnapshot(*m_scene);
    }

    bool DocumentFacade::applyChanges(const ApplyChanges& changes)
    {
        if (!m_scene)
        {
            setError("Document is closed");
            return false;
        }

        if (!PyFacade::applyChanges(*m_scene, changes))
        {
            setError("Failed to apply scene changes");
            return false;
        }

        m_lastError.clear();
        return true;
    }

    size_t DocumentFacade::entityCount() const
    {
        return m_scene ? m_scene->getEntityCount() : 0;
    }

    std::optional<EntitySnapshot> DocumentFacade::getEntity(EntityRef ref) const
    {
        if (!m_scene || !ref.valid())
        {
            return std::nullopt;
        }

        Eg::AprEntity* entity = m_scene->findAprEntityById(ref.id);
        if (!entity)
        {
            return std::nullopt;
        }

        return exportEntity(*entity);
    }

    bool DocumentFacade::save(const std::string& /*path*/)
    {
        setError("Document.save is not implemented yet");
        return false;
    }

    void DocumentFacade::close()
    {
        if (m_scene)
        {
            m_scene->clearScene();
        }
        m_path.clear();
    }

    void DocumentFacade::setError(std::string message)
    {
        m_lastError = std::move(message);
    }
}  // namespace PyFacade