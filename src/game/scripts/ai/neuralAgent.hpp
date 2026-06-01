#pragma once
#include "math.hpp"
#include "world/behaviour.hpp"
#include <cstdint>
#include <string>

class NeuralAgent : public Behaviour
{
  public:
    Matrix m_w1;
    Matrix m_b1;
    Matrix m_w2;
    Matrix m_b2;

    Matrix m_hidden;
    Matrix m_output;

    double m_fitness = 0.0;

    NeuralAgent(uint32_t inputs, uint32_t hidden, uint32_t outputs)
        : m_w1(hidden, inputs), m_b1(hidden, 1), m_w2(outputs, hidden),
          m_b2(outputs, 1), m_hidden(hidden, 1), m_output(outputs, 1)
    {
        m_w1.randomize();
        m_b1.randomize();
        m_w2.randomize();
        m_b2.randomize();
    }
    virtual ~NeuralAgent() override = default;

    bool saveToFile(const std::string &filename) const;
    bool loadFromFile(const std::string &filename);

    const Matrix predict(const Matrix &input);
};
