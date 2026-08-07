#pragma once

#include "PyFacade/FacadeTypes.h"

#include <memory>
#include <string>

namespace Eg
{
    class SceneManager;
}

/**
 * @brief Python 绑定门面层
 *
 * ABI 说明：PyBindCore 通过 PyFacade 值类型（Vec2/EntityRef/EntitySnapshot 等）
 * 隔离 Engine C++ 类型，Engine 类型不直接暴露到 Python 层。
 * DocumentFacade 内部持有 Eg::SceneManager，但仅在同编译器/同 CRT 体系下使用。
 * 如需跨编译器调用，应通过 C ABI facade 而非直接绑定 C++ 类。
 */
namespace PyFacade
{
    class DocumentFacade
    {
    public:
        static std::shared_ptr<DocumentFacade> create();
        static std::pair<std::shared_ptr<DocumentFacade>, std::string> open(const std::string& path);

        bool isValid() const;
        std::string path() const;
        std::string lastError() const;

        EntityRef addPoint(const Vec2& point);
        EntityRef addLine(const Vec2& start, const Vec2& end);
        EntityRef addCircle(const Vec2& center, double radius);

        bool removeEntity(EntityRef ref);
        std::vector<EntityRef> queryBox(const BBox2& box, bool containedOnly) const;

        SceneSnapshot exportSnapshot() const;
        bool applyChanges(const ApplyChanges& changes);

        size_t entityCount() const;
        std::optional<EntitySnapshot> getEntity(EntityRef ref) const;

        bool save(const std::string& path);
        void close();

        ~DocumentFacade();

    private:
        DocumentFacade();
        explicit DocumentFacade(std::unique_ptr<Eg::SceneManager> scene, std::string path);

        void setError(std::string message);

        std::unique_ptr<Eg::SceneManager> m_scene;
        std::string m_path;
        std::string m_lastError;
    };
}
