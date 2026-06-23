#include "neuralAgent.hpp"
#include "scripts/ai/math.hpp"
#include <fstream>
#include <iostream>

const Matrix NeuralAgent::predict(const Matrix &input)
{
    if (m_layers.empty())
    {
        return input;
    }

    Matrix *in_ptr = const_cast<Matrix *>(&input);
    Matrix *out_ptr = &m_output;

    for (size_t i = 0; i < m_layers.size(); ++i)
    {
        Multiply(m_layers[i].m_weights, *in_ptr, *out_ptr);
        Add(*out_ptr, m_layers[i].m_biases, *out_ptr);

        bool isLastLayer = (i == m_layers.size() - 1);
        if (!isLastLayer || m_squashOutput)
        {
            TanhInPlace(*out_ptr);
        }

        in_ptr = out_ptr;
        out_ptr = (out_ptr == &m_output) ? &m_hidden : &m_output;
    }

    if (in_ptr != &m_output)
    {
        m_output = *in_ptr;
    }

    return m_output;
}

bool NeuralAgent::saveToFile(const std::string &filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        return false;
    }

    file << m_layers.size() << "\n";

    auto writeMatrix = [&file](const Matrix &m)
    {
        file << m.m_rows << " " << m.m_cols << "\n";
        for (double val : m.m_data)
        {
            file << val << " ";
        }
        file << "\n";
    };

    for (const auto &layer : m_layers)
    {
        writeMatrix(layer.m_weights);
        writeMatrix(layer.m_biases);
    }

    return true;
}

bool NeuralAgent::loadFromFile(const std::string &filename, bool squashOutput)
{
    m_squashOutput = squashOutput;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        return false;
    }

    size_t numLayers;
    if (!(file >> numLayers))
    {
        return false;
    }

    m_layers.clear();

    auto readMatrix = [&file](Matrix &m)
    {
        int rows, cols;
        file >> rows >> cols;
        m.m_rows = rows;
        m.m_cols = cols;
        m.m_data.resize(rows * cols);
        for (double &val : m.m_data)
        {
            file >> val;
        }
    };

    for (size_t i = 0; i < numLayers; ++i)
    {
        Layer layer(1, 1);
        readMatrix(layer.m_weights);
        readMatrix(layer.m_biases);
        m_layers.push_back(std::move(layer));
    }

    return true;
}
