#pragma once
#include "scripts/ai/math.hpp"
#include "scripts/ai/neuralAgent.hpp"
#include "trainerScene.hpp"
#include <random>

class GeneticTrainer
{
  public:
    GeneticTrainer();

    void evolvePopulation(TrainerScene &scene);

  private:
    std::mt19937 m_rng;

    float m_mutationRate = 0.05f; // Change of mutation
    float m_mutationPower = 0.2f; // Mutation strength
    uint16_t m_elitismCount = 10; // How many top agents aren't changed
    uint32_t m_generation = 0;

    // Size of the pool of agents from which best agent will be chosen
    int m_tournamentSize = 5;

    NeuralAgent *tournamentSelect(const std::vector<NeuralAgent *> &agents,
                                  int tournamentSize);
    void crossoverMatrix(const Matrix &parentA, const Matrix &parentB,
                         Matrix &child);
    void mutateMatrix(Matrix &matrix);
};
