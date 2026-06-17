#pragma once
#include "math.hpp"
#include "world/behaviour.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct Layer
{
    Matrix m_weights;
    Matrix m_biases;

    Layer(uint32_t out, uint32_t in) : m_weights(out, in), m_biases(out, 1)
    {
        m_weights.randomize();
        m_biases.randomize();
    }
};

class NeuralAgent : public Behaviour
{
  public:
    std::vector<Layer> m_layers;

    Matrix m_hidden;
    Matrix m_output;

    double m_fitness = 0.0;

    NeuralAgent(const std::vector<uint32_t> &layerSizes)
        : m_hidden(1, 1), m_output(1, 1)
    {
        for (size_t i = 1; i < layerSizes.size(); ++i)
        {
            m_layers.emplace_back(layerSizes[i], layerSizes[i - 1]);
        }
    }

    NeuralAgent(uint32_t inputs, uint32_t hidden, uint32_t outputs)
        : NeuralAgent(std::vector<uint32_t>{inputs, hidden, outputs})
    {
    }

    virtual ~NeuralAgent() override = default;

    bool saveToFile(const std::string &filename) const;
    bool loadFromFile(const std::string &filename);

    const Matrix predict(const Matrix &input);
};
