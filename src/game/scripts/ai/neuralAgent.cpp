#include "neuralAgent.hpp"
#include "scripts/ai/math.hpp"
#include <fstream>
#include <iostream>

const Matrix NeuralAgent::predict(const Matrix &input)
{
    Multiply(m_w1, input, m_hidden);
    Add(m_hidden, m_b1, m_hidden);
    SigmoidInPlace(m_hidden);

    Multiply(m_w2, m_hidden, m_output);
    Add(m_output, m_b2, m_output);
    SigmoidInPlace(m_output);

    return m_output;
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
            std::cerr << "Error: Wrong matrix size in the file! Expected "
                      << m.m_rows << "x" << m.m_cols << ". Received: " << rows
                      << "x" << cols << "\n";
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
