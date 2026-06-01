#pragma once
#include <memory>
#include <type_traits>
#include <vector>

#include "behaviour.hpp"
#include "component.hpp"

inline size_t GetNextComponentTypeID()
{
    static size_t typeID = 0;
    return typeID++;
}

template <typename T> inline size_t GetComponentTypeID() noexcept
{
    static size_t typeID = GetNextComponentTypeID();
    return typeID;
}

class Scene;

class Entity
{
  public:
    Entity(Scene *scene) : m_scene(scene) {}

    template <typename T, typename... Args> T &AddComponent(Args &&...args);

    template <typename BaseType, typename ActualType, typename... Args>
    ActualType &AddComponentAs(Args &&...args);

    template <typename T> T *GetComponent()
    {
        size_t typeID = GetComponentTypeID<T>();

        if (typeID >= m_componentArray.size())
        {
            return nullptr;
        }

        return static_cast<T *>(m_componentArray[typeID]);
    }

    void notifyTriggerEnter(Collider *otherCollider)
    {
        for (Behaviour *script : m_behaviours)
            script->onTriggerEnter(otherCollider);
    }

    void notifyCollisionEnter(Collider *otherCollider)
    {
        for (Behaviour *script : m_behaviours)
            script->onCollisionEnter(otherCollider);
    }

  private:
    std::vector<std::unique_ptr<Component>> m_components;
    std::vector<Behaviour *> m_behaviours;
    std::vector<Component *> m_componentArray;
    Scene *m_scene;
};

#include "world/scene.hpp"

template <typename T, typename... Args> T &Entity::AddComponent(Args &&...args)
{
    auto newComponent = std::make_unique<T>(std::forward<Args>(args)...);
    newComponent->m_entity = this;
    T *rawPointer = newComponent.get();

    m_components.push_back(std::move(newComponent));

    size_t typeID = GetComponentTypeID<T>();

    if (typeID >= m_componentArray.size())
    {
        m_componentArray.resize(typeID + 1, nullptr);
    }

    m_componentArray[typeID] = rawPointer;

    if constexpr (std::is_base_of_v<Behaviour, T>)
    {
        m_scene->addBehaviour(rawPointer);
        m_behaviours.push_back(rawPointer);
        rawPointer->onStart();
    }

    return *rawPointer;
}
template <typename BaseType, typename ActualType, typename... Args>
ActualType &Entity::AddComponentAs(Args &&...args)
{
    static_assert(std::is_base_of_v<BaseType, ActualType>,
                  "ActualType must inherit from BaseType!");

    auto newComponent =
        std::make_unique<ActualType>(std::forward<Args>(args)...);
    newComponent->m_entity = this;
    ActualType *rawPointer = newComponent.get();

    m_components.push_back(std::move(newComponent));

    size_t baseTypeID = GetComponentTypeID<BaseType>();

    if (baseTypeID >= m_componentArray.size())
    {
        m_componentArray.resize(baseTypeID + 1, nullptr);
    }
    m_componentArray[baseTypeID] = static_cast<BaseType *>(rawPointer);

    size_t actualTypeID = GetComponentTypeID<ActualType>();

    if (actualTypeID >= m_componentArray.size())
    {
        m_componentArray.resize(actualTypeID + 1, nullptr);
    }
    m_componentArray[actualTypeID] = rawPointer;

    if constexpr (std::is_base_of_v<Behaviour, ActualType>)
    {
        m_scene->addBehaviour(rawPointer);
        m_behaviours.push_back(rawPointer);
        rawPointer->onStart();
    }

    return *rawPointer;
}
