#pragma once

#include <cstdint>
#include <vector>
class Matrix
{
  public:
    Matrix(uint32_t rows, uint32_t cols)
        : m_rows(rows), m_cols(cols), m_data(rows * cols, 0.0)
    {
    }

    double &operator()(uint32_t row, uint32_t col)
    {
        return m_data[row * m_cols + col];
    }

    const double &operator()(uint32_t row, uint32_t col) const
    {
        return m_data[row * m_cols + col];
    }

    void randomize();

    uint32_t m_rows;
    uint32_t m_cols;
    std::vector<double> m_data;
};

void Multiply(const Matrix &A, const Matrix &B, Matrix &out);
void Add(const Matrix &A, const Matrix &B, Matrix &out);

double Sigmoid(double num);
Matrix Sigmoid(const Matrix &m);
void SigmoidInPlace(Matrix &m);
void TanhInPlace(Matrix &m);
