#pragma once
#include <cstdint>
#include <glm/glm.hpp>

#include "world/component.hpp"

enum class ColliderType
{
    Sphere,
    Box,
    Halfspace,
    Plane
};

enum CollisionLayer
{
    CAT_GROUND = 1 << 0,
    CAT_PLAYER = 1 << 1,
    CAT_ENEMY = 1 << 2,
    CAT_BALL = 1 << 3
};

#define AI_TRAINING
struct Collider : public Component
{
  public:
    Collider(ColliderType type, bool isTrigger, float restitution = 0.2f,
             float friction = 0.9f)
        : m_type(type), m_isTrigger(isTrigger), m_restitution(restitution),
          m_friction(friction)
    {
    }
    virtual ~Collider() override = default;

    float m_restitution = 0.3f;
    float m_friction = 0.9f;
    ColliderType m_type;
    bool m_isTrigger = false;
    uint64_t m_layer = UINT64_MAX;
    uint64_t m_mask = UINT64_MAX;

#ifdef AI_TRAINING
    int m_arenaId = -1;
#endif
};

struct TransformableCollider : public Collider
{
  public:
    TransformableCollider(ColliderType type, const glm::mat4 &offset,
                          bool isTrigger)
        : Collider(type, isTrigger), m_offset(offset)
    {
    }
    glm::vec3 getAxis(int index) const;

    glm::mat4 m_offset = glm::mat4(1.0f);
    glm::mat4 m_worldTransform = glm::mat4(1.0f);
};

struct HalfspaceCollider : public Collider
{
  public:
    HalfspaceCollider(glm::vec3 normal, float offset, bool isTrigger = false)
        : Collider(ColliderType::Halfspace, isTrigger), m_normal(normal),
          m_offset(offset)
    {
    }

    glm::vec3 m_normal = glm::vec3(0.0f, 1.0f, 0.0f);
    float m_offset = 0.0f;
};

struct SphereCollider : public TransformableCollider
{
  public:
    SphereCollider(float radius = 1.0f,
                   const glm::mat4 &offset = glm::mat4(1.0f),
                   bool isTrigger = false)
        : TransformableCollider(ColliderType::Sphere, offset, isTrigger),
          m_radius(radius)
    {
    }

    float m_radius;
};

struct BoxCollider : public TransformableCollider
{
  public:
    BoxCollider(glm::vec3 halfSize = glm::vec3(1.0f),
                const glm::mat4 &offset = glm::mat4(1.0f),
                bool isTrigger = false)
        : TransformableCollider(ColliderType::Box, offset, isTrigger),
          m_halfSize(halfSize)
    {
    }

    glm::vec3 m_halfSize;
};
