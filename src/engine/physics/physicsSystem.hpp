#pragma once
#include "contact.hpp"
#include "world/components/collider.hpp"
#include "world/components/rigidbody.hpp"
#include "world/entity.hpp"

class PhysicsSystem
{
  public:
    bool sphereAndSphere(SphereCollider &one, SphereCollider &two,
                         Rigidbody *rbA, Rigidbody *rbB);
    bool sphereAndHalfspace(SphereCollider &sphere, HalfspaceCollider &plane,
                            Rigidbody *rbA, Rigidbody *rbB);
    bool boxAndSphere(BoxCollider &box, SphereCollider &sphere, Rigidbody *rbA,
                      Rigidbody *rbB);
    bool boxAndBox(BoxCollider &boxA, BoxCollider &boxB, Rigidbody *rbA,
                   Rigidbody *rbB);
    bool boxAndHalfspaceSimple(const BoxCollider &box,
                               const HalfspaceCollider &plane);
    bool boxAndHalfspace(BoxCollider &box, HalfspaceCollider &plane,
                         Rigidbody *rbA, Rigidbody *rbB);

    void setBodyData(Contact &contact, const Collider &colA,
                     const Collider &colB, Rigidbody *rbA, Rigidbody *rbB);
    void swapBodies(Contact &contact);
    void generateContacts(std::vector<std::unique_ptr<Entity>> &entities);
    std::vector<Contact> getContacts() { return m_contacts; }
    void resolveContacts(float deltaTime);

    /// @brief Handles position change in collision resolution loop.
    void adjustPositions();

    /// @brief Handles velocity change in collision resolution loop.
    /// @param deltaTime Time between simulation steps in seconds.
    void adjustVelocities(float deltaTime);

    void update(const std::vector<std::unique_ptr<Entity>> &entities,
                float deltaTime);

    void notifyTrigger(Collider *colliderA, Collider *colliderB);
    void notifyColliision(Collider *colliderA, Collider *colliderB);

  private:
    struct PhysicsNode
    {
        Collider *col;
        Rigidbody *rb;
    };
    std::vector<Contact> m_contacts;
    std::vector<PhysicsNode> m_physicsNodes;
    float m_positionEpsilon = 0.01f;
    float m_velocityEpsilon = 0.01f;
    unsigned int m_positionIterations = 1;
    unsigned int m_velocityIterations = 1;

    /// @brief Casts box onto an axis.
    /// @param box box
    /// @param axis axis
    /// @return Bounds of the box on the axis.
    float transformToAxis(const BoxCollider &box, const glm::vec3 &axis);

    /// @brief Finds whether two boxes collide on provided axis.
    /// @param boxA first box
    /// @param boxB second box
    /// @param axis axis
    /// @param toCenter vector from second box to first box
    /// @param index index of the axis
    /// @param smallestPenetration smallest penetration that function will
    /// compare to
    /// @param smallestCase index of axis with smallest penetration
    /// @return Whether there was a penetration.
    bool tryAxis(const BoxCollider &boxA, const BoxCollider &boxB,
                 glm::vec3 axis, const glm::vec3 &toCenter, unsigned int index,
                 float &smallestPenetration, unsigned int &smallestCase);

    /// @brief Calculates how much two boxes penetrate on given axis.
    /// @param boxA first box
    /// @param boxB second box
    /// @param axis axis
    /// @param toCenter direction vector from boxB to boxA
    /// @return Penetration between two boxes.
    float penetrationOnAxis(const BoxCollider &boxA, const BoxCollider &boxB,
                            const glm::vec3 &axis, const glm::vec3 &toCenter);

    /// @brief This function is called when there is vertex-face or face-face
    /// collision.
    /// @param boxA first box
    /// @param boxB second box
    /// @param toCenter direction vector from boxB to boxA
    /// @param best index of the axis
    /// @param pen penetration
    void fillPointFaceBoxBox(BoxCollider &boxA, BoxCollider &boxB,
                             Rigidbody *rbA, Rigidbody *rbB,
                             const glm::vec3 &toCenter, unsigned int best,
                             float pen);

    /// @brief Finds closest point between edges of two boxes
    /// @param pOne point one
    /// @param dOne normalized direction vector of edge one
    /// @param oneSize half-size of edge one
    /// @param pTwo point two
    /// @param dTwo normalized direction vector of edge two
    /// @param twoSize half-size of edge two
    /// @param useOne which point to use if the math will go out of bounds
    /// @return point closest between edges of two boxes
    glm::vec3 contactPoint(const glm::vec3 &pOne, const glm::vec3 &dOne,
                           float oneSize, const glm::vec3 &pTwo,
                           const glm::vec3 &dTwo, float twoSize, bool useOne);
};
