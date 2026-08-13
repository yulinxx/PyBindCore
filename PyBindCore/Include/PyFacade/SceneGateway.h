#pragma once

// ABI 说明：SceneGateway 提供 SceneManager/SyEntity 到 PyFacade 值类型的桥接函数。
// 属于 PyBindCore 内部 C++ ABI 层，仅供同编译器/同 CRT 体系内部使用。

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
}  // namespace PyFacade
