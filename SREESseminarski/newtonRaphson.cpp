#include "systemModel.h"
#include "newtonRaphson.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <vector>


/// <summary>
///  Matrix and vector product operator overload
/// </summary>
/// <param name="std::vector<std::vector<T>>">Vector of vector type</param>
/// <param name="std::vector<T>">Vector type</param>
/// <returns>Vector that is the result of matrix and vector product</returns>
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

/// <summary>
///  Unary minus sign matrix operator overload
/// </summary>
/// <param name="std::vector<std::vector<T>>">Matrix of type</param>
/// <returns>Reverse sign elements of matrix</returns>
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


/// <summary>
///  Absolute value of vector
/// </summary>
/// <param name="std::vector<double>">Vector of double elements</param>
/// <returns>Absolute value of elements in the argument vector</returns>
std::vector<double> absVector(const std::vector<double>& vec)
{
	std::vector<double> abs_vec;
	for (int i = 0; i < vec.size(); i++)
		abs_vec.push_back(std::abs(vec[i]));
	return abs_vec;
}


/// <summary>
///  Cofactor of the matrix
/// </summary>
/// <param name="std::vector<std::vector<double>>">Matrix to get cofactor from</param>
/// <param name="std::vector<std::vector<double>>">Cofactor matrix</param>
/// <param name="int">Row of the cofactor that needs to be found</param>
/// <param name="int">Column of the cofactor that needs to be found</param>
/// <param name="int">Size of square matrix</param>
/// <returns></returns>
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


/// <summary>
///  Determinant of matrix
/// </summary>
/// <param name="std::vector<std::vector<double>>">Matrix to get determinant from</param>
/// <param name="int">Size of square matrix</param>
/// <returns>Double value of determinant</returns>
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

/// <summary>
///  Adjoint matrix
/// </summary>
/// <param name="std::vector<std::vector<double>>">Matrix to get adjoint from</param>
/// <param name="std::vector<std::vector<double>>">Referece to adjoint matrix</param>
/// <returns></returns>
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


/// <summary>
///  Inverse matrix
/// </summary>
/// <param name="std::vector<std::vector<double>>">Matrix to get inverse from</param>
/// <param name="double">Sinuglarity check</param>
/// <returns>Matrix that is inverse from the first argument</returns>
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

/// <summary>
///  Newton Raphson method
/// </summary>
/// <param name="SystemModel::SystemModel">System model</param>
/// <param name="int">Maximum number of iterations</param>
/// <param name="double">Maximum tolerance</param>
/// <param name="std::vector<double>">Starting solution vector</param>
/// <param name="std::vector<double>">Reference to solution vector</param>
/// <param name="double">Reference to tolerance achieved</param>
/// <param name="int">Reference to number of iterations preformed</param>
/// <returns>Int value that shows if the system converges or not. Returns 1 if converges, returns 0 if it does not</returns>
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
			std::cout << "Algorithm converges in " << iter << " iterations, with error " << err << std::endl;
			return 1;
		}
		if (iter == maxNumberOfIter)
		{
			throw std::length_error("Maximum number of iterations reached.");
		}
		iter++;
		
	}
}