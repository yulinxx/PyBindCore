#include "PyFacade/SceneGateway.h"

#include "Engine/SyEntity/SyEntity2D.h"
#include "Engine2D/Core/SceneManager.h"
#include "Engine2D/SyEntity/SyLine.h"
#include "Engine2D/SyEntity/SyPoint.h"
#include "Engine2D/SyEntity/SyCircle.h"
#include "Engine2D/SyEntity/SyPolygon.h"

#include "Ut/Vec.h"

#include <unordered_set>

namespace PyFacade
{
    namespace
    {
        Vec2 toVec2(const Ut::Vec2d& v)
        {
            return Vec2{ v.x(), v.y() };
        }

        BBox2 toBBox2(const Ut::BBox2d& box)
        {
            if (!box.isValid())
            {
                return BBox2{};
            }

            return BBox2{ box.minPt.x(), box.minPt.y(), box.maxPt.x(), box.maxPt.y() };
        }

        // 与 DocumentFacade 的同名转换函数区分：unity 合批后两个匿名命名空间会并入同一
        // 翻译单元，同名定义触发 C2084「已有主体」。
        Ut::BBox2d toUtBBox2Scene(const BBox2& box)
        {
            return Ut::BBox2d(Ut::Vec2d(box.minX, box.minY), Ut::Vec2d(box.maxX, box.maxY));
        }

        std::unique_ptr<Eg::SyEntity> entityFromSnapshot(const EntitySnapshot& snapshot)
        {
            if (snapshot.type == "point")
            {
                auto point = std::make_unique<Eg::SyPoint>(Ut::Vec2d(snapshot.basePoint.x, snapshot.basePoint.y));
                point->id = snapshot.id;
                return point;
            }

            if (snapshot.type == "line")
            {
                std::vector<Ut::Vec2d> points;
                points.reserve(snapshot.points.size());
                for (const Vec2& p : snapshot.points)
                {
                    points.emplace_back(p.x, p.y);
                }

                if (points.size() < 2)
                {
                    return nullptr;
                }

                auto line = std::make_unique<Eg::SyLine>(points);
                line->id = snapshot.id;
                return line;
            }

            if (snapshot.type == "circle")
            {
                auto circle = std::make_unique<Eg::SyCircle>();
                circle->basePoint = Ut::Vec2d(snapshot.basePoint.x, snapshot.basePoint.y);
                circle->dRadius = snapshot.radius;
                circle->id = snapshot.id;
                return circle;
            }

            if (snapshot.type == "polygon")
            {
                auto polygon = std::make_unique<Eg::SyPolygon>();
                polygon->basePoint = Ut::Vec2d(snapshot.basePoint.x, snapshot.basePoint.y);
                polygon->bClosed = snapshot.closed;
                auto& verts = polygon->verticesMutable();
                verts.reserve(snapshot.points.size());
                for (const Vec2& p : snapshot.points)
                {
                    verts.emplace_back(p.x, p.y);
                }
                polygon->id = snapshot.id;
                return polygon;
            }

            return nullptr;
        }
    }  // namespace

    std::optional<EntitySnapshot> exportEntity(const Eg::SyEntity& entity)
    {
        EntitySnapshot snapshot;
        snapshot.id = entity.id;
        snapshot.type = entityTypeToString(entity.eType);

        // basePoint / bClosed 不在 SyEntity 基类上：Engine 侧已把 2D 语义字段下移到
        // SyEntity2D 中间层（Engine/SyEntity/SyEntity2D.h:28-29），3D 图元不携带这两个字段。
        // 只有 2D 图元才需要取，且必须先按类型确认再转换，避免对 3D 图元错误下转。
        switch (entity.eType)
        {
        case Eg::EType::POINT:
        case Eg::EType::LINE:
        case Eg::EType::CIRCLE:
        case Eg::EType::POLYGON:
        {
            const auto& entity2d = static_cast<const Eg::SyEntity2D&>(entity);
            snapshot.basePoint = toVec2(entity2d.basePoint);
            snapshot.closed = entity2d.bClosed;
            break;
        }

        default:
            break;
        }

        switch (entity.eType)
        {
        case Eg::EType::POINT:
            return snapshot;

        case Eg::EType::LINE:
        {
            const auto& line = static_cast<const Eg::SyLine&>(entity);
            snapshot.points.reserve(line.pointRef().size());
            for (const Ut::Vec2d& p : line.pointRef())
            {
                snapshot.points.push_back(toVec2(p));
            }
            return snapshot;
        }

        case Eg::EType::CIRCLE:
        {
            const auto& circle = static_cast<const Eg::SyCircle&>(entity);
            snapshot.radius = circle.dRadius;
            return snapshot;
        }

        case Eg::EType::POLYGON:
        {
            const auto& polygon = static_cast<const Eg::SyPolygon&>(entity);
            const auto& verts = polygon.vertices();
            snapshot.points.reserve(verts.size());
            for (const Ut::Vec2d& p : verts)
            {
                snapshot.points.push_back(toVec2(p));
            }
            return snapshot;
        }

        default:
            return std::nullopt;
        }
    }

    SceneSnapshot exportSnapshot(const Eg::SceneManager& scene)
    {
        SceneSnapshot snapshot;
        snapshot.entityCount = scene.getEntityCount();

        Ut::BBox2d bounds;
        bool hasBounds = false;

        for (Eg::SyEntity* entity : scene.getAllEntities())
        {
            if (!entity)
            {
                continue;
            }

            const auto exported = exportEntity(*entity);
            if (!exported.has_value())
            {
                continue;
            }

            snapshot.entities.push_back(exported.value());

            const Ut::BBox2d entityBox = entity->getBbox();
            if (entityBox.isValid())
            {
                if (!hasBounds)
                {
                    bounds = entityBox;
                    hasBounds = true;
                }
                else
                {
                    bounds.expand(entityBox);
                }
            }
        }

        snapshot.bounds = toBBox2(bounds);
        return snapshot;
    }

    bool applyChanges(Eg::SceneManager& scene, const ApplyChanges& changes)
    {
        if (!changes.remove.empty())
        {
            std::vector<Eg::SyEntity*> entitiesToRemove;
            entitiesToRemove.reserve(changes.remove.size());
            for (Eg::EntityId id : changes.remove)
            {
                if (Eg::SyEntity* entity = scene.findEntityById(id))
                {
                    entitiesToRemove.push_back(entity);
                }
            }
            scene.deleteEntitiesBatch(entitiesToRemove);
        }

        if (!changes.add.empty())
        {
            std::vector<std::unique_ptr<Eg::SyEntity>> entitiesToAdd;
            entitiesToAdd.reserve(changes.add.size());
            // 本批已占用的 id：只查场景看不到同批里还没落库的兄弟图元
            std::unordered_set<Eg::EntityId> claimedIds;
            claimedIds.reserve(changes.add.size());
            for (const EntitySnapshot& item : changes.add)
            {
                std::unique_ptr<Eg::SyEntity> entity = entityFromSnapshot(item);
                if (!entity)
                {
                    return false;
                }

                if (entity->id != 0 && scene.findEntityById(entity->id))
                {
                    return false;
                }

                // id=0 或与场景/本批冲突时分配一个真正空闲的 id。
                // 直接取 getNextPersistentId() 会撞上已打开文档里占用的低位 id，
                // insertEntitiesPreserveId 静默丢弃，脚本侧却收到成功。
                if (entity->id == 0 || claimedIds.find(entity->id) != claimedIds.end())
                {
                    Eg::EntityId candidate = 0;
                    for (size_t attempt = 0; attempt < 1000000 && candidate == 0; ++attempt)
                    {
                        const Eg::EntityId generated =
                            Eg::EntityIdGenerator::instance().getNextPersistentId();
                        if (!scene.findEntityById(generated) && claimedIds.find(generated) == claimedIds.end())
                        {
                            candidate = generated;
                        }
                    }
                    if (candidate == 0)
                    {
                        return false;
                    }
                    entity->id = candidate;
                }

                claimedIds.insert(entity->id);
                entitiesToAdd.push_back(std::move(entity));
            }
            scene.insertEntitiesPreserveId(std::move(entitiesToAdd));
        }

        return true;
    }
}  // namespace PyFacade