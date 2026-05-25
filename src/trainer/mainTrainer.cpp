#include "core/application.hpp"
#include "geneticTrainer.hpp"
#include "trainerScene.hpp"
#include <iostream>

int main()
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
    static const uint32_t FRAMES_PER_GENERATION = 3600;

    GeneticTrainer trainer;

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
                trainer.evolvePopulation(*scenePtr);
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
                    trainer.evolvePopulation(*scenePtr);

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
