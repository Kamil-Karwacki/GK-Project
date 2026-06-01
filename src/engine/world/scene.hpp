#pragma once
#include <memory>
#include <vector>

class Entity;
class Behaviour;
class PhysicsSystem;

class Scene
{
  public:
    Scene();
    virtual ~Scene();
    virtual void init();
    virtual void update(float deltaTime);
    virtual void fixedUpdate(float deltaTime);
    virtual void draw();

    Entity &createEntity();

    void addBehaviour(Behaviour *behaviour);

  protected:
    std::vector<std::unique_ptr<Entity>> m_entities;
    std::vector<Behaviour *> m_activeBehaviours;

    std::unique_ptr<PhysicsSystem> m_physicsSystem;
};
