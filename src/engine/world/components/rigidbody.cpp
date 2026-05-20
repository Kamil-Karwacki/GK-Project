#include "rigidbody.hpp"

glm::mat3 Rigidbody::createBoxInverseInertiaTensor(float mass, float dx,
                                                   float dy, float dz)
{
    if (mass <= 0.0f)
        return glm::mat3(0.0f);

    float ix = (1.0f / 12.0f) * mass * (dy * dy + dz * dz);
    float iy = (1.0f / 12.0f) * mass * (dx * dx + dz * dz);
    float iz = (1.0f / 12.0f) * mass * (dx * dx + dy * dy);

    glm::mat3 invInertia(0.0f);
    invInertia[0][0] = 1.0f / ix;
    invInertia[1][1] = 1.0f / iy;
    invInertia[2][2] = 1.0f / iz;

    return invInertia;
}

glm::mat3 Rigidbody::createSphereInverseInertiaTensor(float mass, float radius)
{
    if (mass <= 0.0f)
        return glm::mat3(0.0f);

    float invI = 2.5f / (mass * radius * radius);

    glm::mat3 invInertia(0.0f);

    invInertia[0][0] = invI;
    invInertia[1][1] = invI;
    invInertia[2][2] = invI;

    return invInertia;
}
