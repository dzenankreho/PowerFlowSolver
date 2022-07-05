#include "systemModel.h"
#include "newtonRaphson.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <vector>


template <typename T>
std::vector<T> operator *(const std::vector<std::vector<T>>& matrix, const std::vector<T>& vector)
{
	if (matrix[0].size() != vector.size())
		throw std::range_error("Incapable size.");

	std::vector<T> ret;
	for (int i = 0; i < matrix.size(); i++)
	{
		T c(0);
		for (int j = 0; j < vector.size(); j++)
			c += matrix[i][j] * vector[j];
		ret.push_back(c);
	}
	return ret;
}

template <typename T>
std::vector<std::vector<T>> operator -(const std::vector<std::vector<T>>& matrix)
{
	std::vector<std::vector<T>> ret;
	for (int i = 0; i < matrix.size(); i++)
	{
		std::vector<T> vec;
		for (int j = 0; j < matrix.size(); j++)
		{
			vec.push_back(-matrix[i][j]);
		}
		ret.push_back(vec);
	}
	return ret;
}



std::vector<double> absVector(const std::vector<double>& vec)
{
	std::vector<double> abs_vec;
	for (int i = 0; i < vec.size(); i++)
		abs_vec.push_back(std::abs(vec[i]));
	return abs_vec;
}

void cofactor(const std::vector<std::vector<double>>& matrix, std::vector<std::vector<double>>& t, int p, int q, int n) {
	int i(0), j(0);
	for (int r = 0; r < n; r++)
	{
		for (int c = 0; c < n; c++)
		{
			if (r != p && c != q)
			{
				t[i][j++] = matrix[r][c];
				if (j == n - 1)
				{
					j = 0; i++;
				}
			}
		}
	}
}

double determinant(std::vector<std::vector<double>> matrix, int n)
{
	double D(0);
	if (n == 1)
		return matrix[0][0];
	
	// Store cofactors
	std::vector<std::vector<double>> t(n,std::vector<double>(n)); 
	
	// Sign multiplier 
	int s = 1; 
		for (int f = 0; f < n; f++) {
			cofactor(matrix, t, 0, f, n);
			D += s * matrix[0][f] * determinant(t, n - 1);
			s = -s;
		}
	return D;
}

void adjoint(const std::vector<std::vector<double>>& matrix, std::vector<std::vector<double>>& adj)
{
	if (matrix.size() == 1) {
		adj[0][0] = 1;
		return;
	}
	int s = 1;
	std::vector<std::vector<double>> t(matrix.size(), std::vector<double>(matrix.size()));
	for (int i = 0; i < matrix.size(); i++) {
		for (int j = 0; j < matrix.size(); j++) {

			//To get cofactor of M[i][j]
			cofactor(matrix, t, i, j, matrix.size());

			//sign of adj[j][i] positive if sum of row and column indexes is even.
			s = ((i + j) % 2 == 0) ? 1 : -1; 
			
			//Interchange rows and columns to get the transpose of the cofactor matrix
			adj[j][i] = (s) * (determinant(t, matrix.size() - 1)); 
		}
	}
}

std::vector<std::vector<double>> inverseMatrix(const std::vector<std::vector<double>>& matrix, double eps = 1e-10)
{
	double det(determinant(matrix,matrix.size()));
	if (std::fabs(det) < eps)
		throw std::range_error("Singular matrix");

	std::vector<std::vector<double>> inv(matrix.size(), std::vector<double>(matrix.size()));
	std::vector<std::vector<double>> adj(matrix.size(), std::vector<double>(matrix.size()));
	adjoint(matrix, adj);
	for (int i = 0; i < matrix.size(); i++)
		for (int j = 0; j < matrix.size(); j++) 
			inv[i][j] = adj[i][j] / float(det);
	return inv;

}

int newtonRaphson(SystemModel::SystemModel sm, int maxNumberOfIter, double eps, std::vector<double> x0, std::vector<double> &x, double &err, int &iter)
{
	x.resize(x0.size());
	for (int i = 0; i < x.size(); i++)
		x[i] = x0[i];

	iter = 0;


	while (1)
	{

		std::vector <double> functionsCalculated;
		std::vector <std::vector<double> > jacobianCalculated;
		for (int i = 1; i <= sm.getNumberOfBuses(); i++)
		{
			functionsCalculated.push_back(sm.getBusFunctions(i).first(x));
			functionsCalculated.push_back(sm.getBusFunctions(i).second(x));
			std::vector<double> pb_vecP, pb_vecQ;
			for (int j = 0; j < 2*sm.getNumberOfBuses(); j++) {
				pb_vecP.push_back(sm.getDerivativesOfBusFunctions(i).first[j](x));
				pb_vecQ.push_back(sm.getDerivativesOfBusFunctions(i).second[j](x));
			}
			jacobianCalculated.push_back(pb_vecP);
			jacobianCalculated.push_back(pb_vecQ);
		}

		std::vector <std::vector<double>> inv_jac(inverseMatrix(jacobianCalculated));

		std::vector<double> dx(-inv_jac * functionsCalculated);

		for (int i = 0; i < x.size(); i++)
			x[i] += dx[i];

		auto abs_vec(absVector(dx));
		err = *std::max_element(abs_vec.begin(), abs_vec.end());
		std::cout << iter << " " << err<<std::endl;
		if (eps >= err)
		{
			std::cout << "Algorithm converges in " << iter << " iterations, with error " << err;
			return 1;
		}
		if (iter == maxNumberOfIter)
		{
			throw std::length_error("Maximum number of iterations reached.");
		}
		iter++;
		
	}
}