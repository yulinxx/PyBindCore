#pragma once

#include "PyFacade/FacadeTypes.h"

namespace Eg
{
    class SceneManager;
}

namespace PyFacade
{
    SceneSnapshot exportSnapshot(const Eg::SceneManager& scene);
    bool applyChanges(Eg::SceneManager& scene, const ApplyChanges& changes);
    std::optional<EntitySnapshot> exportEntity(const Eg::SyEntity& entity);
}
