#pragma once
#include "matchArena.hpp"
#include "scripts/ai/math.hpp"
#include "scripts/ai/neuralAgent.hpp"
#include <cstdint>
#include <iostream>
#include <random>

class GeneticTrainer
{
  public:
    GeneticTrainer(float mutationRate = 0.05f, float mutationPower = 0.2f,
                   uint16_t elitismCount = 30)
        : m_mutationRate(mutationRate), m_mutationPower(mutationPower),
          m_elitismCount(elitismCount)
    {
        std::random_device rd;
        m_rng = std::mt19937(rd());
        std::cout << "Genetic algorithm parameters\n"
                     "mutation rate: "
                  << mutationRate
                  << "\n"
                     "mutation power: "
                  << mutationPower
                  << "\n"
                     "elitism count: "
                  << elitismCount << "\n";
    }

    void evolvePopulation(std::vector<MatchArena> &arenas);

  private:
    std::mt19937 m_rng;

    float m_mutationRate = 0.05f; // Change of mutation
    float m_mutationPower = 0.9f; // Mutation strength
    
    // Decay parameters
    float m_minMutationRate = 0.005f;
    float m_minMutationPower = 0.1f;
    float m_decayFactor = 0.99f;
    
    uint16_t m_elitismCount = 30; // How many top agents aren't changed
    uint32_t m_generation = 0;

    // Size of the pool of agents from which best agent will be chosen
    int m_tournamentSize = 12;

    NeuralAgent *tournamentSelect(const std::vector<NeuralAgent *> &agents,
                                  int tournamentSize);
    void crossoverMatrix(const Matrix &parentA, const Matrix &parentB,
                         Matrix &child);
    void mutateMatrix(Matrix &matrix);
};
