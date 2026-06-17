#include "core/application.hpp"
#include "geneticTrainer.hpp"
#include "trainerScene.hpp"
#include <cstdint>
#include <iostream>

int main(int argc, char *argv[])
{
    Application app;

    auto trainerScene = std::make_unique<TrainerScene>(app.getWhiteTexture());
    TrainerScene *scenePtr = trainerScene.get();
    app.loadScene(std::move(trainerScene));

    glfwSetInputMode(app.m_window->getNativeWindow(), GLFW_CURSOR,
                     GLFW_CURSOR_DISABLED);

    bool isVisualMode = true;
    const double FIXED_DT = 1.0 / 60.0;
    uint32_t frameCount = 0;
    static const uint32_t FRAMES_PER_GENERATION = 1800;

    float mutationRate = 0.05f;
    float mutationPower = 0.2f;
    uint16_t elitismCount = 30;
    if (argc == 4)
    {
        mutationRate = std::stof(argv[1]);
        mutationPower = std::stof(argv[2]);
        elitismCount = std::stoi(argv[3]);
    }
    GeneticTrainer trainer(mutationRate, mutationPower, elitismCount);

    while (!app.m_window->ShouldClose())
    {
        app.GetInput().update();
        glfwPollEvents();

        if (app.GetInput().isKeyPressed(GLFW_KEY_V))
        {
            std::cout << "Toggling visual mode\n";
            isVisualMode = !isVisualMode;
        }

        if (isVisualMode)
        {
            app.m_activeScene->update(FIXED_DT);
            app.m_activeScene->fixedUpdate(FIXED_DT);

            frameCount++;
            if (frameCount >= FRAMES_PER_GENERATION)
            {
                trainer.evolvePopulation(scenePtr->m_arenas);
                frameCount = 0;
                std::cout << "Generation finished. Mutating weights.\n";
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            app.m_activeScene->draw();
            app.m_window->SwapBuffers();
        }
        else
        {
            for (int i = 0; i < 1000; ++i)
            {
                app.m_activeScene->update(FIXED_DT);
                app.m_activeScene->fixedUpdate(FIXED_DT);

                frameCount++;

                if (frameCount >= FRAMES_PER_GENERATION)
                {
                    trainer.evolvePopulation(scenePtr->m_arenas);

                    frameCount = 0;
                    std::cout << "Generation finished. Mutating weights.\n";
                }
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            app.m_activeScene->draw();
            app.m_window->SwapBuffers();
        }
    }
    return 0;
}
