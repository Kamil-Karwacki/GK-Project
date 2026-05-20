#include "Matrix.hpp"
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>

void Matrix::randomize()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(0, 1);
    for (auto &val : m_data)
    {
        val = d(gen);
    }
}

void Multiply(const Matrix &A, const Matrix &B, Matrix &out)
{
    if (A.m_cols != B.m_rows)
    {
        throw std::invalid_argument(
            "Wrong matrix dimensions for multiplication!");
    }

    if (&A == &out || &B == &out)
    {
        Matrix temp(A.m_rows, B.m_cols);

        Multiply(A, B, temp);

        out = std::move(temp);
        return;
    }

    out.m_rows = A.m_rows;
    out.m_cols = B.m_cols;
    out.m_data.assign(A.m_rows * B.m_cols, 0.0);

    for (uint32_t i = 0; i < A.m_rows; ++i)
    {
        for (uint32_t k = 0; k < A.m_cols; ++k)
        {
            double a_ik = A(i, k);
            for (uint32_t j = 0; j < B.m_cols; ++j)
            {
                out(i, j) += a_ik * B(k, j);
            }
        }
    }
}

void Add(const Matrix &A, const Matrix &B, Matrix &out)
{
    if (A.m_rows != B.m_rows || A.m_cols != B.m_cols)
    {
        throw std::invalid_argument("Wrong matrix dimensions for addition!");
    }

    out.m_rows = A.m_rows;
    out.m_cols = A.m_cols;
    out.m_data.resize(A.m_rows * A.m_cols);

    for (uint32_t i = 0; i < A.m_data.size(); i++)
    {
        out.m_data[i] = A.m_data[i] + B.m_data[i];
    }
}

double Sigmoid(double num) { return 1.0 / (1.0 + exp(-num)); }

Matrix Sigmoid(const Matrix &m)
{
    Matrix result(m.m_rows, m.m_cols);

    for (uint32_t i = 0; i < m.m_data.size(); ++i)
    {
        result.m_data[i] = 1.0 / (1.0 + std::exp(-m.m_data[i]));
    }
    return result;
}
