#include "geneticTrainer.hpp"
#include "scripts/ai/neuralAgent.hpp"
#include "world/entity.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

NeuralAgent *
GeneticTrainer::tournamentSelect(const std::vector<NeuralAgent *> &agents,
                                 int tournamentSize)
{
    std::uniform_int_distribution<int> dist(0, agents.size() - 1);
    NeuralAgent *best = nullptr;
    for (int t = 0; t < tournamentSize; t++)
    {
        NeuralAgent *candidate = agents[dist(m_rng)];
        if (!best || candidate->m_fitness > best->m_fitness)
            best = candidate;
    }
    return best;
}

void GeneticTrainer::evolvePopulation(std::vector<MatchArena> &arenas)
{
    std::vector<NeuralAgent *> allAgents;
    allAgents.reserve(arenas.size() * 2);

    for (auto &arena : arenas)
    {
        NeuralAgent *agentA = arena.m_playerA->GetComponent<NeuralAgent>();
        agentA->m_fitness = arena.m_fitnessA;
        allAgents.push_back(agentA);

        NeuralAgent *agentB = arena.m_playerB->GetComponent<NeuralAgent>();
        agentB->m_fitness = arena.m_fitnessB;
        allAgents.push_back(agentB);
    }

    for (size_t i = 0; i < allAgents.size(); ++i)
    {
        int similarCount = 0;
        for (size_t j = 0; j < allAgents.size(); ++j)
        {
            if (i == j)
                continue;
            float diff = 0.0f;
            const auto& biasesA = allAgents[i]->m_layers.back().m_biases;
            const auto& biasesB = allAgents[j]->m_layers.back().m_biases;
            for (size_t k = 0; k < biasesA.m_data.size(); ++k)
                diff += std::abs(biasesA.m_data[k] - biasesB.m_data[k]);
            if (diff < 0.5f)
                similarCount++;
        }
        allAgents[i]->m_fitness -= similarCount * 5.0f;
    }

    std::sort(allAgents.begin(), allAgents.end(),
              [](NeuralAgent *a, NeuralAgent *b)
              { return a->m_fitness > b->m_fitness; });

    std::cout << "--- GENERATION " << m_generation << " FINISHED ---\n";
    std::cout << "Best fitness:  " << allAgents[0]->m_fitness << "\n";
    std::cout << "Worst fitness: " << allAgents.back()->m_fitness << "\n";
    std::cout << "Mutation Rate: " << m_mutationRate << "\n";
    std::cout << "Mutation Power:" << m_mutationPower << "\n";
    m_generation++;

    m_mutationRate = std::max(m_minMutationRate, m_mutationRate * m_decayFactor);
    m_mutationPower = std::max(m_minMutationPower, m_mutationPower * m_decayFactor);

    allAgents[0]->saveToFile("best_brain.txt");

    float top10Sum = 0;
    for (int i = 0; i < 10 && i < (int)allAgents.size(); i++)
        top10Sum += allAgents[i]->m_fitness;
    std::cout << "Average score of top 10 agents: " << (top10Sum / 10.0f)
              << "\n\n";

    std::vector<std::vector<Layer>> nextGeneration;
    nextGeneration.reserve(allAgents.size());

    //  Elitism - best agents aren't affected
    for (uint16_t i = 0; i < m_elitismCount && i < allAgents.size(); ++i)
    {
        nextGeneration.push_back(allAgents[i]->m_layers);
    }

    // Crossover + mutation
    for (size_t i = m_elitismCount; i < allAgents.size(); ++i)
    {
        NeuralAgent *parentA = tournamentSelect(allAgents, m_tournamentSize);
        NeuralAgent *parentB = tournamentSelect(allAgents, m_tournamentSize);

        std::vector<Layer> childLayers;
        childLayers.reserve(parentA->m_layers.size());

        for (size_t L = 0; L < parentA->m_layers.size(); ++L)
        {
            // Create a child layer matching the dimensions of the parent layer
            Layer childLayer(parentA->m_layers[L].m_weights.m_rows, parentA->m_layers[L].m_weights.m_cols);
            
            // Crossing entire neurons instead of singular weights
            // so that network get full strategies/behaviours
            // instead of partial changes
            crossoverMatrix(parentA->m_layers[L].m_weights, parentB->m_layers[L].m_weights, childLayer.m_weights);
            crossoverMatrix(parentA->m_layers[L].m_biases, parentB->m_layers[L].m_biases, childLayer.m_biases);

            mutateMatrix(childLayer.m_weights);
            mutateMatrix(childLayer.m_biases);

            childLayers.push_back(std::move(childLayer));
        }

        nextGeneration.push_back(std::move(childLayers));
    }

    std::shuffle(nextGeneration.begin(), nextGeneration.end(), m_rng);

    for (size_t i = 0; i < allAgents.size(); ++i)
    {
        allAgents[i]->m_layers = nextGeneration[i];
        allAgents[i]->m_fitness = 0.0f;
    }

    for (auto &arena : arenas)
    {
        arena.m_fitnessA = 0.0f;
        arena.m_fitnessB = 0.0f;
        arena.resetPositions();
    }
}

void GeneticTrainer::crossoverMatrix(const Matrix &parentA,
                                     const Matrix &parentB, Matrix &child)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (uint32_t row = 0; row < child.m_rows; ++row)
    {
        bool useA = dist(m_rng) > 0.5f;
        for (uint32_t col = 0; col < child.m_cols; ++col)
        {
            uint32_t idx = row * child.m_cols + col;
            child.m_data[idx] =
                useA ? parentA.m_data[idx] : parentB.m_data[idx];
        }
    }
}

void GeneticTrainer::mutateMatrix(Matrix &matrix)
{
    std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
    std::normal_distribution<double> noiseDist(0.0, m_mutationPower);

    for (size_t i = 0; i < matrix.m_data.size(); ++i)
    {
        if (chanceDist(m_rng) < m_mutationRate)
        {
            matrix.m_data[i] += noiseDist(m_rng);
        }
    }
}
