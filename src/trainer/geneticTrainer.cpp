#include "geneticTrainer.hpp"
#include "scripts/ai/neuralAgent.hpp"
#include "world/entity.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

GeneticTrainer::GeneticTrainer()
{
    std::random_device rd;
    m_rng = std::mt19937(rd());
}

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

    std::sort(allAgents.begin(), allAgents.end(),
              [](NeuralAgent *a, NeuralAgent *b)
              { return a->m_fitness > b->m_fitness; });

    std::cout << "--- GENERATION " << m_generation << " FINISHED ---\n";
    std::cout << "Best fitness:  " << allAgents[0]->m_fitness << "\n";
    std::cout << "Worst fitness: " << allAgents.back()->m_fitness << "\n";
    m_generation++;

    allAgents[0]->saveToFile("best_brain.txt");

    float top10Sum = 0;
    for (int i = 0; i < 10 && i < (int)allAgents.size(); i++)
        top10Sum += allAgents[i]->m_fitness;
    std::cout << "Average score of top 10 agents: " << (top10Sum / 10.0f)
              << "\n\n";

    struct Brain
    {
        Matrix w1;
        Matrix b1;
        Matrix w2;
        Matrix b2;

        Brain(uint32_t inputs, uint32_t hidden, uint32_t outputs)
            : w1(hidden, inputs), b1(hidden, 1),
              w2(outputs, hidden), b2(outputs, 1)
        {
        }
    };

    std::vector<Brain> nextGeneration;
    nextGeneration.reserve(allAgents.size());

    //  Elitism - best agents aren't affected
    for (uint16_t i = 0; i < m_elitismCount && i < allAgents.size(); ++i)
    {
        Brain b(allAgents[i]->m_w1.m_cols, allAgents[i]->m_w1.m_rows, allAgents[i]->m_w2.m_rows);
        b.w1 = allAgents[i]->m_w1;
        b.b1 = allAgents[i]->m_b1;
        b.w2 = allAgents[i]->m_w2;
        b.b2 = allAgents[i]->m_b2;
        nextGeneration.push_back(std::move(b));
    }

    // Crossover + mutation
    for (size_t i = m_elitismCount; i < allAgents.size(); ++i)
    {
        NeuralAgent *parentA = tournamentSelect(allAgents, m_tournamentSize);
        NeuralAgent *parentB = tournamentSelect(allAgents, m_tournamentSize);

        Brain child(parentA->m_w1.m_cols, parentA->m_w1.m_rows, parentA->m_w2.m_rows);

        // Crossing entire neurons instead of singular weights
        // so that network get full strategies/behaviours
        // instead of partial chagnes
        crossoverMatrix(parentA->m_w1, parentB->m_w1, child.w1);
        crossoverMatrix(parentA->m_b1, parentB->m_b1, child.b1);
        crossoverMatrix(parentA->m_w2, parentB->m_w2, child.w2);
        crossoverMatrix(parentA->m_b2, parentB->m_b2, child.b2);

        mutateMatrix(child.w1);
        mutateMatrix(child.b1);
        mutateMatrix(child.w2);
        mutateMatrix(child.b2);

        nextGeneration.push_back(std::move(child));
    }

    std::shuffle(nextGeneration.begin(), nextGeneration.end(), m_rng);

    for (size_t i = 0; i < allAgents.size(); ++i)
    {
        allAgents[i]->m_w1 = nextGeneration[i].w1;
        allAgents[i]->m_b1 = nextGeneration[i].b1;
        allAgents[i]->m_w2 = nextGeneration[i].w2;
        allAgents[i]->m_b2 = nextGeneration[i].b2;
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
