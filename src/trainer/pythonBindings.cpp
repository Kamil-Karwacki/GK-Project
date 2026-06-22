#include "headlessApplication.hpp"
#include "headlessTrainerScene.hpp"
#include "scripts/enemyController.hpp"
#include "scripts/footballer.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <vector>

namespace py = pybind11;

class SoccerEnv
{
  public:
    SoccerEnv()
    {
        m_app = std::make_unique<HeadlessApplication>();
        auto scene = std::make_unique<HeadlessTrainerScene>();
        m_scene = scene.get();
        m_app->loadScene(std::move(scene));

        for (auto &arena : m_scene->m_arenas)
        {
            // Disable the C++ internal auto-reset so Python can handle it
            // properly
            arena.m_autoReset = false;

            auto *enemyControllerA =
                arena.m_playerA->GetComponent<EnemyController>();
            if (enemyControllerA)
            {
                enemyControllerA->m_pythonControlled = true;
            }
            auto *enemyControllerB =
                arena.m_playerB->GetComponent<EnemyController>();
            if (enemyControllerB)
            {
                enemyControllerB->m_pythonControlled = true;
            }
        }
        m_prevFitness.resize(num_envs(), 0.0f);
        m_frameCount.resize(m_scene->m_arenas.size(), 0);
    }

    int num_envs() const { return m_scene->m_arenas.size() * 2; }
    int observationSize() const { return 43; }
    int actionSize() const { return 6; }

    py::array_t<float> reset()
    {
        auto result = py::array_t<float>({num_envs(), observationSize()});
        auto buf = result.mutable_unchecked<2>();

        int num_arenas = m_scene->m_arenas.size();
        for (int i = 0; i < num_arenas; i++)
        {
            m_scene->m_arenas[i].resetPositions();
            m_scene->m_arenas[i].m_fitnessA = 0.0f;
            m_scene->m_arenas[i].m_fitnessB = 0.0f;
            m_frameCount[i] = 0;
            m_prevFitness[i] = 0.0f;
            m_prevFitness[i + num_arenas] = 0.0f;
        }
        for (int i = 0; i < num_envs(); i++)
        {
            for (int j = 0; j < observationSize(); j++)
            {
                buf(i, j) = 0.0f;
            }
        }
        return result;
    }

    py::tuple step(py::array_t<float> actions)
    {
        auto acts_buf = actions.unchecked<2>();
        int num_arenas = m_scene->m_arenas.size();

        // Apply actions to all arenas
        for (int i = 0; i < num_envs(); i++)
        {
            int arena_idx = i % num_arenas;
            bool is_b = (i >= num_arenas);
            Entity *playerEnt = is_b ? m_scene->m_arenas[arena_idx].m_playerB
                                     : m_scene->m_arenas[arena_idx].m_playerA;

            auto *footballer = playerEnt->GetComponent<Footballer>();
            if (footballer)
            {
                footballer->m_input.x = acts_buf(i, 0);
                footballer->m_input.y = acts_buf(i, 1);
                footballer->m_jump = acts_buf(i, 2) > 0.5f;
                if (acts_buf(i, 3) > 0.5f)
                    footballer->kickBall();
            }

            auto *enemyController = playerEnt->GetComponent<EnemyController>();
            if (enemyController)
            {
                enemyController->m_lastTurnYaw = acts_buf(i, 4) * 2.0f;
                enemyController->m_lastTurnPitch = acts_buf(i, 5) * 2.0f;
            }
        }

        // Step physics and C++ scripts
        m_app->step(1.0f / 60.0f);

        auto obs_result = py::array_t<float>({num_envs(), observationSize()});
        auto obs_buf = obs_result.mutable_unchecked<2>();
        auto rewards_result = py::array_t<float>(num_envs());
        auto rewards_buf = rewards_result.mutable_unchecked<1>();
        auto dones_result = py::array_t<bool>(num_envs());
        auto dones_buf = dones_result.mutable_unchecked<1>();

        py::list infos;

        for (int i = 0; i < num_envs(); i++)
        {
            int arena_idx = i % num_arenas;
            bool is_b = (i >= num_arenas);
            MatchArena &arena = m_scene->m_arenas[arena_idx];

            if (!is_b)
            {
                m_frameCount[arena_idx]++;
            }

            float current_fitness = is_b ? arena.m_fitnessB : arena.m_fitnessA;
            float reward = current_fitness - m_prevFitness[i];
            m_prevFitness[i] = current_fitness;
            rewards_buf(i) = reward;

            bool done =
                arena.m_needsReset || m_frameCount[arena_idx] >= MAX_FRAMES;
            dones_buf(i) = done;

            py::dict info;

            Entity *playerEnt = is_b ? arena.m_playerB : arena.m_playerA;
            auto *enemyController = playerEnt->GetComponent<EnemyController>();

            if (done)
            {
                auto terminal_obs = py::array_t<float>(observationSize());
                auto term_buf = terminal_obs.mutable_unchecked<1>();
                if (enemyController)
                {
                    const auto &mat = enemyController->getInputMatrix();
                    for (int j = 0; j < observationSize(); j++)
                    {
                        term_buf(j) = static_cast<float>(mat.m_data[j]);
                    }
                }
                info["terminal_observation"] = terminal_obs;

                for (int j = 0; j < observationSize(); j++)
                {
                    obs_buf(i, j) = 0.0f; // New reset observation
                }
            }
            else
            {
                if (enemyController)
                {
                    const auto &mat = enemyController->getInputMatrix();
                    for (int j = 0; j < observationSize(); j++)
                    {
                        obs_buf(i, j) = static_cast<float>(mat.m_data[j]);
                    }
                }
                else
                {
                    for (int j = 0; j < observationSize(); j++)
                    {
                        obs_buf(i, j) = 0.0f;
                    }
                }
            }
            infos.append(info);
        }

        // Now reset the arenas that are done
        for (int i = 0; i < num_arenas; i++)
        {
            MatchArena &arena = m_scene->m_arenas[i];
            bool done = arena.m_needsReset || m_frameCount[i] >= MAX_FRAMES;
            if (done)
            {
                arena.resetPositions();
                arena.m_fitnessA = 0.0f;
                arena.m_fitnessB = 0.0f;
                m_frameCount[i] = 0;
                m_prevFitness[i] = 0.0f;
                m_prevFitness[i + num_arenas] = 0.0f;
            }
        }

        return py::make_tuple(obs_result, rewards_result, dones_result, infos);
    }

  private:
    static constexpr int MAX_FRAMES =
        1800; //  Matches C++ genetic algorithm duration

    std::unique_ptr<HeadlessApplication> m_app;
    HeadlessTrainerScene *m_scene = nullptr;
    std::vector<int> m_frameCount;
    std::vector<float> m_prevFitness;
};

PYBIND11_MODULE(soccer_engine, m)
{
    py::class_<SoccerEnv>(m, "SoccerEnv")
        .def(py::init<>())
        .def("reset", &SoccerEnv::reset)
        .def("step", &SoccerEnv::step)
        .def("num_envs", &SoccerEnv::num_envs)
        .def("observation_size", &SoccerEnv::observationSize)
        .def("action_size", &SoccerEnv::actionSize);
}
