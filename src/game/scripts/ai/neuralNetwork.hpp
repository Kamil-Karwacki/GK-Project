#pragma once
#include "Matrix.hpp"
#include <cstdint>

class NeuralNetwork
{
  public:
    NeuralNetwork(const std::vector<int> &sizes)
    {

        for (uint32_t i = 1; i < sizes.size(); i++)
        {
            Matrix b(sizes[i], 1);
            b.randomize();
            m_biases.push_back(b);

            Matrix w(sizes[i], sizes[i - 1]);
            w.randomize();
            m_weights.push_back(w);
        }
    }

    Matrix feedForward(Matrix a)
    {
        for (uint32_t i = 0; i < m_biases.size(); ++i)
        {
            Matrix z(m_weights[i].m_rows, a.m_cols);
            Multiply(m_weights[i], a, z);
            Add(m_biases[i], z, z);

            a = Sigmoid(z);
        }
        return a;
    }

  private:
    std::vector<Matrix> m_biases;
    std::vector<Matrix> m_weights;
};
