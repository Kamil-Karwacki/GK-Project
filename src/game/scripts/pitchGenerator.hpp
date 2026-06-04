#pragma once
#include "world/entity.hpp"
#include <functional>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Shader;
class Scene;

namespace PitchGenerator
{
struct PitchConfig
{
    glm::vec2 pitchSize = glm::vec2(115, 74) * 1.4f;
    float wallHeight = 4.0f;
    float bannerLength = 32.0f;
    glm::vec3 gateSize = glm::vec3(30.0f, 11.0f, 11.0f);
    float gateThickness = 0.7f;
    float groundThickness = 0.5f;
};

struct GeneratedGatesInfo
{
    glm::vec3 gateAPos;
    glm::vec3 gateBPos;
};

void generatePitch(std::vector<std::unique_ptr<Entity>> &targetEntities,
                   Scene *scene, const glm::vec3 &offset, Shader *defaultShader,
                   const PitchConfig &config = PitchConfig());

GeneratedGatesInfo
generateGates(std::vector<std::unique_ptr<Entity>> &targetEntities,
              Scene *scene, const glm::vec3 &offset, Shader *defaultShader,
              std::function<void()> onGoalScoredA = nullptr,
              std::function<void()> onGoalScoredB = nullptr,
              const PitchConfig &config = PitchConfig());
} // namespace PitchGenerator
