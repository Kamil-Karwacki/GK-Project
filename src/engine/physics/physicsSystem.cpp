#include "physicsSystem.hpp"

#include "physics/contact.hpp"
#include "world/component.hpp"
#include "world/components/collider.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "core/debug.hpp"

bool PhysicsSystem::sphereAndSphere(SphereCollider &one, SphereCollider &two,
                                    Rigidbody *rbA, Rigidbody *rbB)
{
    glm::vec3 positionOne = one.getAxis(3);
    glm::vec3 positionTwo = two.getAxis(3);

    glm::vec3 midline = positionOne - positionTwo;
    float size = glm::length(midline);

    if (size <= 0.0f || size >= one.m_radius + two.m_radius)
        return false;

    bool isTrigger = one.m_isTrigger || two.m_isTrigger;

    if (isTrigger)
    {
        notifyTrigger(&one, &two);
        return true;
    }

    glm::vec3 normal = midline * (1.0f / size);

    Contact contact;
    contact.m_contactNormal = normal;
    contact.m_contactPoint = positionOne + midline * 0.5f;
    contact.m_penetration = (one.m_radius + two.m_radius - size);

    setBodyData(contact, one, two, rbA, rbB);

    m_contacts.push_back(contact);

    notifyColliision(&one, &two);
    return true;
}

bool PhysicsSystem::sphereAndHalfspace(SphereCollider &sphere,
                                       HalfspaceCollider &plane, Rigidbody *rbA,
                                       Rigidbody *rbB)
{
    glm::vec3 position = sphere.getAxis(3);

    float ballDistance =
        glm::dot(plane.m_normal, position) - sphere.m_radius - plane.m_offset;

    if (ballDistance >= 0)
        return false;

    bool isTrigger = sphere.m_isTrigger || plane.m_isTrigger;
    if (isTrigger)
    {
        notifyTrigger(&sphere, &plane);
        return true;
    }

    Contact contact;
    contact.m_contactNormal = plane.m_normal;
    contact.m_penetration = -ballDistance;
    contact.m_contactPoint =
        position - plane.m_normal * (ballDistance + sphere.m_radius);

    setBodyData(contact, sphere, plane, rbA, rbB);

    m_contacts.push_back(contact);
    notifyColliision(&sphere, &plane);
    return true;
}

bool PhysicsSystem::boxAndSphere(BoxCollider &box, SphereCollider &sphere,
                                 Rigidbody *rbA, Rigidbody *rbB)
{
    glm::vec3 sphereCenter = sphere.getAxis(3);
    glm::vec3 relCenter = glm::vec3(glm::inverse(box.m_worldTransform) *
                                    glm::vec4(sphereCenter, 1.0f));

    if (fabsf(relCenter.x) - sphere.m_radius > box.m_halfSize.x ||
        fabsf(relCenter.y) - sphere.m_radius > box.m_halfSize.y ||
        fabsf(relCenter.z) - sphere.m_radius > box.m_halfSize.z)
    {
        return false;
    }

    bool isTrigger = box.m_isTrigger || sphere.m_isTrigger;
    if (isTrigger)
    {
        notifyTrigger(&box, &sphere);
        return true;
    }

    glm::vec3 closestPoint(0.0f, 0.0f, 0.0f);
    float distance;

    distance = relCenter.x;
    if (distance > box.m_halfSize.x)
        distance = box.m_halfSize.x;
    if (distance < -box.m_halfSize.x)
        distance = -box.m_halfSize.x;
    closestPoint.x = distance;

    distance = relCenter.y;
    if (distance > box.m_halfSize.y)
        distance = box.m_halfSize.y;
    if (distance < -box.m_halfSize.y)
        distance = -box.m_halfSize.y;
    closestPoint.y = distance;

    distance = relCenter.z;
    if (distance > box.m_halfSize.z)
        distance = box.m_halfSize.z;
    if (distance < -box.m_halfSize.z)
        distance = -box.m_halfSize.z;
    closestPoint.z = distance;

    distance = glm::length2(closestPoint - relCenter);
    if (distance > sphere.m_radius * sphere.m_radius)
        return false;

    glm::vec3 closestPtWorld =
        glm::vec3(box.m_worldTransform * glm::vec4(closestPoint, 1.0f));

    Contact contact;

    // Precaution when spheres center would be inside box (NaN)
    if (distance * distance < 0.0001f)
    {
        glm::vec3 boxCenter = box.getAxis(3);

        contact.m_contactNormal = glm::normalize(sphereCenter - boxCenter);

        if (glm::any(glm::isnan(contact.m_contactNormal)))
        {
            contact.m_contactNormal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        contact.m_penetration = sphere.m_radius;
    }
    else
    {
        contact.m_contactNormal = glm::normalize(closestPtWorld - sphereCenter);
        contact.m_penetration = sphere.m_radius - sqrt(distance);
    }
    contact.m_contactPoint = closestPtWorld;

    setBodyData(contact, box, sphere, rbA, rbB);

    m_contacts.push_back(contact);
    notifyColliision(&box, &sphere);
    return true;
}

float PhysicsSystem::transformToAxis(const BoxCollider &box,
                                     const glm::vec3 &axis)
{
    return box.m_halfSize.x * fabsf(glm::dot(axis, box.getAxis(0))) +
           box.m_halfSize.y * fabsf(glm::dot(axis, box.getAxis(1))) +
           box.m_halfSize.z * fabsf(glm::dot(axis, box.getAxis(2)));
}

#define CHECK_OVERLAP(axis, index)                                             \
    if (!tryAxis(boxA, boxB, (axis), toCenter, (index), pen, best))            \
        return 0;

bool PhysicsSystem::boxAndBox(BoxCollider &boxA, BoxCollider &boxB,
                              Rigidbody *rbA, Rigidbody *rbB)
{
    glm::vec3 toCenter = boxB.getAxis(3) - boxA.getAxis(3);

    float pen = FLT_MAX;
    unsigned int best = 0xffffff;

    // Check axes for all normal vector for each box.
    // Here vertex-face or face-face collision are detected.
    CHECK_OVERLAP(boxA.getAxis(0), 0);
    CHECK_OVERLAP(boxA.getAxis(1), 1);
    CHECK_OVERLAP(boxA.getAxis(2), 2);

    CHECK_OVERLAP(boxA.getAxis(0), 3);
    CHECK_OVERLAP(boxA.getAxis(1), 4);
    CHECK_OVERLAP(boxA.getAxis(2), 5);

    unsigned int bestSingleAxis = best;

    // Check cross products of axes for both boxes.
    // Here edge-edge collision is detected.
    CHECK_OVERLAP(glm::cross(boxA.getAxis(0), boxB.getAxis(0)), 6);
    CHECK_OVERLAP(glm::cross(boxA.getAxis(0), boxB.getAxis(1)), 7);
    CHECK_OVERLAP(glm::cross(boxA.getAxis(0), boxB.getAxis(2)), 8);
    CHECK_OVERLAP(glm::cross(boxA.getAxis(1), boxB.getAxis(0)), 9);
    CHECK_OVERLAP(glm::cross(boxA.getAxis(1), boxB.getAxis(1)), 10);
    CHECK_OVERLAP(glm::cross(boxA.getAxis(1), boxB.getAxis(2)), 11);
    CHECK_OVERLAP(glm::cross(boxA.getAxis(2), boxB.getAxis(0)), 12);
    CHECK_OVERLAP(glm::cross(boxA.getAxis(2), boxB.getAxis(1)), 13);
    CHECK_OVERLAP(glm::cross(boxA.getAxis(2), boxB.getAxis(2)), 14);

    if (best < 3)
    {
        // Box two's vertex is on a face of box one.
        fillPointFaceBoxBox(boxA, boxB, rbA, rbB, toCenter, best, pen);
        return true;
    }
    else if (best < 6)
    {
        // Box one's vertex is one a face of box two.
        fillPointFaceBoxBox(boxA, boxB, rbA, rbB, toCenter * -1.0f, best - 3,
                            pen);
        return true;
    }
    else
    {
        bool isTrigger = boxA.m_isTrigger || boxB.m_isTrigger;
        if (isTrigger)
        {
            notifyTrigger(&boxA, &boxB);
            return true;
        }
        // Edge-edge contact
        best -= 6;
        // Normalize the axes.
        unsigned oneAxisIndex = best / 3;
        unsigned twoAxisIndex = best % 3;
        glm::vec3 oneAxis = boxA.getAxis(oneAxisIndex);
        glm::vec3 twoAxis = boxB.getAxis(twoAxisIndex);
        glm::vec3 axis = glm::cross(oneAxis, twoAxis);
        axis = glm::normalize(axis);

        if (glm::dot(axis, toCenter) > 0)
            axis = axis * -1.0f;

        glm::vec3 ptOnOneEdge = boxA.m_halfSize;
        glm::vec3 ptOnTwoEdge = boxB.m_halfSize;
        for (size_t i = 0; i < 3; i++)
        {
            if (i == oneAxisIndex)
                ptOnOneEdge[i] = 0;
            else if (glm::dot(boxA.getAxis(i), axis) > 0)
                ptOnOneEdge[i] = -ptOnOneEdge[i];

            if (i == twoAxisIndex)
                ptOnTwoEdge[i] = 0;
            else if (glm::dot(boxB.getAxis(i), axis) < 0)
                ptOnTwoEdge[i] = -ptOnTwoEdge[i];
        }

        ptOnOneEdge = boxA.m_worldTransform * glm::vec4(ptOnOneEdge, 1.0f);
        ptOnTwoEdge = boxB.m_worldTransform * glm::vec4(ptOnTwoEdge, 1.0f);

        glm::vec3 vertex = contactPoint(
            ptOnOneEdge, oneAxis, boxA.m_halfSize[oneAxisIndex], ptOnTwoEdge,
            twoAxis, boxB.m_halfSize[twoAxisIndex], bestSingleAxis > 2);

        Contact contact;
        contact.m_penetration = pen;
        contact.m_contactNormal = axis;
        contact.m_contactPoint = vertex;

        setBodyData(contact, boxA, boxB, rbA, rbB);

        m_contacts.push_back(contact);
        notifyColliision(&boxA, &boxB);

        return true;
    }
    return false;
}
#undef CHECK_OVERLAP

glm::vec3 PhysicsSystem::contactPoint(const glm::vec3 &pOne,
                                      const glm::vec3 &dOne, float oneSize,
                                      const glm::vec3 &pTwo,
                                      const glm::vec3 &dTwo, float twoSize,
                                      bool useOne)
{
    glm::vec3 toSt, cOne, cTwo;
    float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
    // Here parametric equation is used:
    // L1(mua) = pOne + mua * dOne
    // L2(mub) = pTwo + muB * dTwo
    // mua, mub - scalars which are calculated later so
    // that distance between L1 and L2 is smallest
    float denom, mua, mub;

    smOne = glm::length2(dOne);
    smTwo = glm::length2(dTwo);
    dpOneTwo = glm::dot(dTwo, dOne);

    toSt = pOne - pTwo; // to start
    dpStaOne = glm::dot(dOne, toSt);
    dpStaTwo = glm::dot(dTwo, toSt);

    denom = smOne * smTwo - dpOneTwo * dpOneTwo;

    // Make sure that there is no divison by 0.
    if (fabsf(denom) < 0.0001f)
        return useOne ? pOne : pTwo;

    mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
    mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

    if (mua > oneSize || mua < -oneSize || mub > twoSize || mub < -twoSize)
    {
        return useOne ? pOne : pTwo;
    }
    else
    {
        cOne = pOne + dOne * mua;
        cTwo = pTwo + dTwo * mub;

        return cOne * 0.5f + cTwo * 0.5f;
    }
}

bool PhysicsSystem::tryAxis(const BoxCollider &boxA, const BoxCollider &boxB,
                            glm::vec3 axis, const glm::vec3 &toCenter,
                            unsigned int index, float &smallestPenetration,
                            unsigned int &smallestCase)
{
    if (glm::length2(axis) < 0.0001f)
        return true;
    axis = glm::normalize(axis);

    float penetration = penetrationOnAxis(boxA, boxB, axis, toCenter);

    if (penetration < 0)
        return false;
    if (penetration < smallestPenetration)
    {
        smallestPenetration = penetration;
        smallestCase = index;
    }
    return true;
}

float PhysicsSystem::penetrationOnAxis(const BoxCollider &boxA,
                                       const BoxCollider &boxB,
                                       const glm::vec3 &axis,
                                       const glm::vec3 &toCenter)
{
    float oneProject = transformToAxis(boxA, axis);
    float twoProject = transformToAxis(boxB, axis);

    float distance = fabsf(glm::dot(toCenter, axis));

    return oneProject + twoProject - distance;
}

void PhysicsSystem::fillPointFaceBoxBox(BoxCollider &boxA, BoxCollider &boxB,
                                        Rigidbody *rbA, Rigidbody *rbB,
                                        const glm::vec3 &toCenter,
                                        unsigned int best, float pen)
{
    bool isTrigger = boxA.m_isTrigger || boxB.m_isTrigger;
    if (isTrigger)
    {
        notifyTrigger(&boxA, &boxB);
        return;
    }

    glm::vec3 normal = boxA.getAxis(best);
    if (glm::dot(boxA.getAxis(best), toCenter) > 0)
    {
        normal = -normal;
    }

    // Check which vertex is colliding.
    glm::vec3 vertex = boxB.m_halfSize;
    if (glm::dot(boxB.getAxis(0), normal) < 0)
        vertex.x = -vertex.x;
    if (glm::dot(boxB.getAxis(1), normal) < 0)
        vertex.y = -vertex.y;
    if (glm::dot(boxB.getAxis(2), normal) < 0)
        vertex.z = -vertex.z;

    Contact contact;
    contact.m_contactNormal = normal;
    contact.m_contactPoint = boxB.m_worldTransform * glm::vec4(vertex, 1.0f);
    contact.m_penetration = pen;

    setBodyData(contact, boxA, boxB, rbA, rbB);

    m_contacts.push_back(contact);
    notifyColliision(&boxA, &boxB);
}

bool PhysicsSystem::boxAndHalfspaceSimple(const BoxCollider &box,
                                          const HalfspaceCollider &plane)
{
    float projectedRadius = transformToAxis(box, plane.m_normal);
    float boxDistance =
        glm::dot(plane.m_normal, box.getAxis(3)) - projectedRadius;

    return boxDistance <= plane.m_offset;
}

bool PhysicsSystem::boxAndHalfspace(BoxCollider &box, HalfspaceCollider &plane,
                                    Rigidbody *rbA, Rigidbody *rbB)
{
    if (!boxAndHalfspaceSimple(box, plane))
        return false;

    bool isTrigger = box.m_isTrigger || plane.m_isTrigger;
    if (isTrigger)
    {
        notifyTrigger(&box, &plane);
        return true;
    }
    static float mults[8][3] = {{1, 1, 1},   {-1, 1, 1},  {1, -1, 1},
                                {-1, -1, 1}, {1, 1, -1},  {-1, 1, -1},
                                {1, -1, -1}, {-1, -1, -1}};

    for (size_t i = 0; i < 8; i++)
    {
        glm::vec3 vertexPos(mults[i][0], mults[i][1], mults[i][2]);
        vertexPos *= box.m_halfSize;
        vertexPos = box.m_worldTransform * glm::vec4(vertexPos, 1.0f);

        float vertexDistance = glm::dot(vertexPos, plane.m_normal);

        if (vertexDistance <= plane.m_offset)
        {
            Contact contact;
            contact.m_contactPoint = vertexPos;
            contact.m_contactNormal = plane.m_normal;
            contact.m_penetration = plane.m_offset - vertexDistance;
            setBodyData(contact, box, plane, rbA, rbB);

            m_contacts.push_back(contact);
        }
    }
    notifyColliision(&box, &plane);
    return true;
}

void PhysicsSystem::setBodyData(Contact &contact, const Collider &colA,
                                const Collider &colB, Rigidbody *rbA,
                                Rigidbody *rbB)
{
    // Make sure that first body is never nullptr
    if (rbA)
    {
        contact.m_body[0] = rbA;
        contact.m_body[1] = rbB;
    }
    else
    {
        contact.m_body[0] = rbB;
        contact.m_body[1] = rbA;
        contact.m_contactNormal *= -1;
    }

    float restitution = (colA.m_restitution + colB.m_restitution) * 0.5f;
    float friction = (colA.m_friction + colB.m_friction) * 0.5f;

    contact.m_restitution = restitution;
    contact.m_friction = friction;
}

void PhysicsSystem::generateContacts(
    std::vector<std::unique_ptr<Entity>> &entities)
{
    m_contacts.clear();
    if (entities.size() < 2)
        return;

    std::vector<PhysicsNode> physicsNodes;
    physicsNodes.reserve(entities.size());

    for (auto &entity : entities)
    {
        Transform *transform = entity->GetComponent<Transform>();
        if (!transform)
            continue;

        Collider *collider = entity->GetComponent<Collider>();

        if (collider)
        {
            Rigidbody *rb = entity->GetComponent<Rigidbody>();
            physicsNodes.push_back({collider, rb});

            if (collider->m_type != ColliderType::Sphere &&
                collider->m_type != ColliderType::Box)
                continue;

            TransformableCollider *transformable =
                static_cast<TransformableCollider *>(collider);

            transformable->m_worldTransform =
                transform->getModelMatrix() * transformable->m_offset;
        }
    }

    for (size_t i = 0; i < physicsNodes.size(); ++i)
    {
        Rigidbody *rbA = physicsNodes[i].rb;
        Collider *colA = physicsNodes[i].col;

        if (!colA)
            continue;

        for (size_t j = i + 1; j < physicsNodes.size(); ++j)
        {
            Rigidbody *rbB = physicsNodes[j].rb;
            Collider *colB = physicsNodes[j].col;

            if (!colB)
                continue;

            if (!rbA && !rbB && !colA->m_isTrigger && !colB->m_isTrigger)
                continue;

            bool aHitsB = (colA->m_mask & colB->m_layer) != 0;
            bool bHitsA = (colB->m_mask & colA->m_layer) != 0;
            if (!aHitsB || !bHitsA)
                continue;

            if (colA->m_type == ColliderType::Sphere &&
                colB->m_type == ColliderType::Sphere)
            {
                auto *sA = static_cast<SphereCollider *>(colA);
                auto *sB = static_cast<SphereCollider *>(colB);
                sphereAndSphere(*sA, *sB, rbA, rbB);
            }
            else if (colA->m_type == ColliderType::Box &&
                     colB->m_type == ColliderType::Sphere)
            {
                auto *bA = static_cast<BoxCollider *>(colA);
                auto *sB = static_cast<SphereCollider *>(colB);
                boxAndSphere(*bA, *sB, rbA, rbB);
            }
            else if (colA->m_type == ColliderType::Sphere &&
                     colB->m_type == ColliderType::Box)
            {
                auto *sA = static_cast<SphereCollider *>(colA);
                auto *bB = static_cast<BoxCollider *>(colB);
                boxAndSphere(*bB, *sA, rbB, rbA);
            }
            else if (colA->m_type == ColliderType::Box &&
                     colB->m_type == ColliderType::Box)
            {
                auto *bA = static_cast<BoxCollider *>(colA);
                auto *bB = static_cast<BoxCollider *>(colB);
                boxAndBox(*bA, *bB, rbA, rbB);
            }
            else if (colA->m_type == ColliderType::Sphere &&
                     colB->m_type == ColliderType::Halfspace)
            {
                auto *sA = static_cast<SphereCollider *>(colA);
                auto *pB = static_cast<HalfspaceCollider *>(colB);
                sphereAndHalfspace(*sA, *pB, rbA, rbB);
            }
            else if (colA->m_type == ColliderType::Halfspace &&
                     colB->m_type == ColliderType::Sphere)
            {
                auto *pA = static_cast<HalfspaceCollider *>(colA);
                auto *sB = static_cast<SphereCollider *>(colB);
                sphereAndHalfspace(*sB, *pA, rbB, rbA);
            }
            else if (colA->m_type == ColliderType::Box &&
                     colB->m_type == ColliderType::Halfspace)
            {
                auto *bA = static_cast<BoxCollider *>(colA);
                auto *pB = static_cast<HalfspaceCollider *>(colB);
                boxAndHalfspace(*bA, *pB, rbA, rbB);
            }
            else if (colA->m_type == ColliderType::Halfspace &&
                     colB->m_type == ColliderType::Box)
            {
                auto *pA = static_cast<HalfspaceCollider *>(colA);
                auto *bB = static_cast<BoxCollider *>(colB);
                boxAndHalfspace(*bB, *pA, rbB, rbA);
            }
        }

        /*for (Contact& c : m_contacts)
        {
            Debug::addLine(c.m_contactPoint, c.m_contactPoint +
        c.m_contactNormal * 5.0f, glm::vec3(1.0f, 1.0f, 1.0f), 0.5f);
        }*/
    }
}

void PhysicsSystem::resolveContacts(float deltaTime)
{
    if (m_contacts.empty())
        return;

    for (Contact &c : m_contacts)
    {
        c.calculateInternals(deltaTime);
    }

    m_positionIterations = m_contacts.size() * 2;
    m_velocityIterations = m_contacts.size() * 2;

    adjustPositions();

    adjustVelocities(deltaTime);

    m_contacts.clear();
}

void PhysicsSystem::adjustPositions()
{
    unsigned int i, index;
    glm::vec3 linearChange[2], angularChange[2];
    float max;
    glm::vec3 deltaPosition;

    unsigned int positionIterationsUsed = 0;
    size_t numContacts = m_contacts.size();
    while (positionIterationsUsed < m_positionIterations)
    {
        max = m_positionEpsilon;
        index = numContacts;
        for (i = 0; i < numContacts; i++)
        {
            if (m_contacts[i].m_penetration > max)
            {
                max = m_contacts[i].m_penetration;
                index = i;
            }
        }
        if (index == numContacts)
            break;

        m_contacts[index].applyPositionChange(linearChange, angularChange, max);

        // If body was moved, then other contact points which involved that body
        // are outdated. Therefore we need to update contacts that belong to
        // affected body.
        for (i = 0; i < numContacts; i++)
        {
            for (size_t b = 0; b < 2; b++)
            {
                if (!m_contacts[i].m_body[b])
                    continue;

                for (size_t d = 0; d < 2; d++)
                {
                    if (m_contacts[i].m_body[b] == m_contacts[index].m_body[d])
                    {
                        deltaPosition =
                            linearChange[d] +
                            glm::cross(
                                angularChange[d],
                                m_contacts[i].m_relativeContactPosition[b]);

                        m_contacts[i].m_penetration +=
                            glm::dot(deltaPosition,
                                     m_contacts[i].m_contactNormal) *
                            (b ? 1.0f : -1.0f);
                    }
                }
            }
        }
        positionIterationsUsed++;
    }
}

void PhysicsSystem::adjustVelocities(float deltaTime)
{
    glm::vec3 velocityChange[2], rotationChange[2];
    glm::vec3 deltaVel;
    float numContacts = m_contacts.size();
    size_t velocityIterationsUsed = 0;

    while (velocityIterationsUsed < m_velocityIterations)
    {
        float max = m_velocityEpsilon;
        size_t index = m_contacts.size();
        for (size_t i = 0; i < numContacts; i++)
        {
            if (m_contacts[i].m_desiredDeltaVelocity > max)
            {
                max = m_contacts[i].m_desiredDeltaVelocity;
                index = i;
            }
        }
        if (index == numContacts)
            break;

        m_contacts[index].applyVelocityChange(velocityChange, rotationChange);

        // Once we apply velocity change, if there are other bodies colliding
        // with affected body we need to update their contact velocities.
        for (size_t i = 0; i < numContacts; i++)
        {
            for (size_t b = 0; b < 2; b++)
            {
                if (!m_contacts[i].m_body[b])
                    continue;

                for (size_t d = 0; d < 2; d++)
                {
                    if (m_contacts[i].m_body[b] == m_contacts[index].m_body[d])
                    {
                        deltaVel =
                            velocityChange[d] +
                            glm::cross(
                                rotationChange[d],
                                m_contacts[i].m_relativeContactPosition[b]);

                        m_contacts[i].m_contactVelocity +=
                            glm::transpose(m_contacts[i].m_contactToWorld) *
                            deltaVel * (b ? -1.0f : 1.0f);
                        m_contacts[i].calculateDesiredDeltaVelocity(deltaTime);
                    }
                }
            }
        }
        velocityIterationsUsed++;
    }
}

void PhysicsSystem::update(const std::vector<std::unique_ptr<Entity>> &entities,
                           float deltaTime)
{
    for (const std::unique_ptr<Entity> &entityPtr : entities)
    {
        Entity *entity = entityPtr.get();
        Rigidbody *rb = entity->GetComponent<Rigidbody>();
        Transform *transform = entity->GetComponent<Transform>();

        if (!rb || !transform)
            continue;
        if (rb->m_inverseMass == 0.0f)
            continue;

        glm::vec3 acceleration = rb->m_forceAcc * rb->m_inverseMass;
        rb->m_velocity += acceleration * deltaTime;

        glm::vec3 angularAcceleration =
            rb->m_invInertiaTensorWorld * rb->m_torqueAcc;
        rb->m_angularVelocity += angularAcceleration * deltaTime;

        rb->m_velocity *= pow(rb->m_linearDamping, deltaTime);
        rb->m_angularVelocity *= pow(rb->m_angularDamping, deltaTime);

        rb->m_lastFrameAcc = acceleration;
        transform->addPosition(rb->m_velocity * deltaTime);
        transform->addRotation(rb->m_angularVelocity * deltaTime);

        // Update inverse inertia tensor world due to objects rotation.
        glm::mat4 modelMatrix = transform->getModelMatrix();
        glm::mat3 rotationMatrix = glm::mat3(modelMatrix);
        rb->m_invInertiaTensorWorld = rotationMatrix * rb->m_invInertiaTensor *
                                      glm::transpose(rotationMatrix);

        rb->m_forceAcc = glm::vec3(0.0f);
        rb->m_torqueAcc = glm::vec3(0.0f);
    }
}

void PhysicsSystem::notifyTrigger(Collider *colliderA, Collider *colliderB)
{
    if (!colliderA || !colliderB)
        return;

    colliderA->m_entity->notifyTriggerEnter(colliderB);
    colliderB->m_entity->notifyTriggerEnter(colliderA);
}

void PhysicsSystem::notifyColliision(Collider *colliderA, Collider *colliderB)
{
    if (!colliderA || !colliderB)
        return;

    colliderA->m_entity->notifyCollisionEnter(colliderB);
    colliderB->m_entity->notifyCollisionEnter(colliderA);
}
