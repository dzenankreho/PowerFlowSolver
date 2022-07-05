#pragma once
#include "systemModel.h"
#include <vector>
#include <math.h>
#include <algorithm>

template <typename T>
std::vector<T> operator *(const std::vector<std::vector<T>>& matrix, const std::vector<T>& vector);
 
template <typename T>
std::vector<std::vector<T>> operator -(const std::vector<std::vector<T>>& matrix);

std::vector<double> absVector(const std::vector<double>& vec);

void cofactor(const std::vector<std::vector<double>>& matrix, std::vector<std::vector<double>>& t, int p, int q, int n);

double determinant(std::vector<std::vector<double>> matrix, int n);

void adjoint(const std::vector<std::vector<double>>& matrix, std::vector<std::vector<double>>& adj);

std::vector<std::vector<double>> inverseMatrix(const std::vector<std::vector<double>>& matrix, double eps);

int newtonRaphson(SystemModel::SystemModel sm, int maxNumberOfIter, double eps, std::vector<double> x0, std::vector<double>& x, double& err, int& iter);
