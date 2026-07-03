#pragma once

#include "Engine/EntityIdGenerator.h"
#include "Engine/SyEntity/SyEntity.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace PyFacade
{
    struct Vec2
    {
        double x = 0.0;
        double y = 0.0;
    };

    struct BBox2
    {
        double minX = 0.0;
        double minY = 0.0;
        double maxX = 0.0;
        double maxY = 0.0;

        bool isValid() const
        {
            return minX <= maxX && minY <= maxY;
        }
    };

    struct EntityRef
    {
        Eg::EntityId id = 0;

        bool valid() const
        {
            return id != 0;
        }

        bool operator==(const EntityRef& other) const
        {
            return id == other.id;
        }

        bool operator!=(const EntityRef& other) const
        {
            return !(*this == other);
        }
    };

    struct EntitySnapshot
    {
        Eg::EntityId id = 0;
        std::string type;
        Vec2 basePoint;
        std::vector<Vec2> points;
        double radius = 0.0;
        bool closed = false;
    };

    struct SceneSnapshot
    {
        std::vector<EntitySnapshot> entities;
        BBox2 bounds;
        size_t entityCount = 0;
    };

    struct ApplyChanges
    {
        std::vector<EntitySnapshot> add;
        std::vector<Eg::EntityId> remove;
    };

    std::string entityTypeToString(Eg::EType type);
}
