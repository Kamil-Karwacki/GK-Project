#include "neuralAgent.hpp"
#include <fstream>
#include <iostream>

Matrix NeuralAgent::predict(const Matrix &input) const
{
    Matrix hidden(m_w1.m_rows, input.m_cols);
    Multiply(m_w1, input, hidden);
    Add(hidden, m_b1, hidden);
    hidden = Sigmoid(hidden);

    Matrix final(m_w2.m_rows, input.m_cols);
    Multiply(m_w2, hidden, final);
    Add(final, m_b2, final);
    final = Sigmoid(final);

    return final;
}

bool NeuralAgent::saveToFile(const std::string &filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Couldn't open file to save neural agent: " << filename
                  << "\n";
        return false;
    }

    auto writeMatrix = [&file](const Matrix &m)
    {
        file << m.m_rows << " " << m.m_cols << "\n";
        for (double val : m.m_data)
        {
            file << val << " ";
        }
        file << "\n";
    };

    writeMatrix(m_w1);
    writeMatrix(m_b1);
    writeMatrix(m_w2);
    writeMatrix(m_b2);

    file.close();
    return true;
}

bool NeuralAgent::loadFromFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to read file with weights: " << filename << "\n";
        return false;
    }

    auto readMatrix = [&file](Matrix &m)
    {
        int rows, cols;
        file >> rows >> cols;

        if (rows != m.m_rows || cols != m.m_cols)
        {
            std::cerr << "Error: Wrong matrix size in the file!\n";
            return;
        }

        for (double &val : m.m_data)
        {
            file >> val;
        }
    };

    readMatrix(m_w1);
    readMatrix(m_b1);
    readMatrix(m_w2);
    readMatrix(m_b2);

    file.close();
    return true;
}
