#include "geneticTrainer.hpp"
#include "headlessApplication.hpp"
#include "headlessTrainerScene.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    HeadlessApplication app;

    auto trainerScene = std::make_unique<HeadlessTrainerScene>();
    HeadlessTrainerScene *scenePtr = trainerScene.get();
    app.loadScene(std::move(trainerScene));

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

    while (true)
    {
        app.step(FIXED_DT);

        frameCount++;

        if (frameCount >= FRAMES_PER_GENERATION)
        {
            trainer.evolvePopulation(scenePtr->m_arenas);

            frameCount = 0;
            std::cout << "Generation finished. Mutating weights.\n";
        }
    }
    return 0;
}
