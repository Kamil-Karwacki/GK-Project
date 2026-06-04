#include "headlessApplication.hpp"
#include "headlessTrainerScene.hpp"
#include "scripts/footballer.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

class SoccerEnv {
public:
  SoccerEnv() {
    /*m_app = std::make_unique<HeadlessApplication>();
    auto scene = std::make_unique<HeadlessTrainerScene>();
    m_scene = scene.get();
    m_app->loadScene(std::move(scene));*/

    m_arena = &m_scene->m_arenas[0];
  }

  py::array_t<float> reset() {
    m_arena->resetPositions();
    m_arena->m_fitnessA = 0.0f;
    m_arena->m_fitnessB = 0.0f;
    m_done = false;
    return getObservations();
  }

  py::tuple step(py::array_t<float> action) {
    applyAction(action);

    m_app->step(1.0f / 60.0f);
    m_frameCount++;

    float reward = computeReward();
    m_done = m_arena->m_needsReset || m_frameCount >= MAX_FRAMES;

    if (m_done)
      m_frameCount = 0;

    return py::make_tuple(getObservations(), reward, m_done, py::dict());
  }

  int observationSize() const { return 38; }
  int actionSize() const { return 4; }

private:
  static constexpr int MAX_FRAMES = 3600;

  std::unique_ptr<HeadlessApplication> m_app;
  HeadlessTrainerScene *m_scene = nullptr;
  MatchArena *m_arena = nullptr;
  int m_frameCount = 0;
  bool m_done = false;

  py::array_t<float> getObservations() {
    /*        auto result = py::array_t<float>(38);
            auto buf = result.mutable_unchecked<1>();

            auto *trans = m_arena->m_playerA->GetComponent<Transform>();
            auto *rb = m_arena->m_playerA->GetComponent<Rigidbody>();
            auto *ballT = m_arena->m_ball->GetComponent<Transform>();
            auto *ballRb = m_arena->m_ball->GetComponent<Rigidbody>();

            return result;*/
  }

  void applyAction(py::array_t<float> action) {
    auto buf = action.unchecked<1>();
    auto *footballer = m_arena->m_playerA->GetComponent<Footballer>();

    footballer->m_input.x = buf(0);
    footballer->m_input.y = buf(1);
    footballer->m_jump = buf(2) > 0.5f;
    if (buf(3) > 0.5f)
      footballer->kickBall();
  }

  float computeReward() {
    float reward = 0.0f;

    auto *ballT = m_arena->m_ball->GetComponent<Transform>();
    glm::vec3 ballPos = ballT->getPosition();

    float currDist = glm::distance(ballPos, m_arena->m_gateBPos);
    float delta = m_prevBallDist - currDist;
    if (delta > 0.0f)
      reward += delta * 1.0f;
    m_prevBallDist = currDist;

    if (m_arena->m_needsReset) {
      // Sprawdź kto strzelił gola
      if (m_arena->m_fitnessA >= 1000.0f)
        reward += 1000.0f;
      if (m_arena->m_fitnessB >= 1000.0f)
        reward -= 1000.0f;
    }

    return reward;
  }

  float m_prevBallDist = 0.0f;
};

PYBIND11_MODULE(soccer_engine, m) {
  py::class_<SoccerEnv>(m, "SoccerEnv")
      .def(py::init<>())
      .def("reset", &SoccerEnv::reset)
      .def("step", &SoccerEnv::step)
      .def("observation_size", &SoccerEnv::observationSize)
      .def("action_size", &SoccerEnv::actionSize);
}
