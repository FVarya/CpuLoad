#include "LSM.h"

LinearSystemOfLSM::LinearSystemOfLSM(std::vector<double> x, std::vector<double> y, int dimension) {
	Dimension = dimension;
	A.resize(Dimension);
	B.resize(Dimension, 0);
	X.resize(Dimension, 0);


	for (int i = 0; i < Dimension; i++)
		A[i].resize(Dimension, 0);

	for (int i = 0; i < Dimension; i++) {
		for (int j = 0; j < x.size(); j++)
		{
			A[0][i] += pow(x[j], i);
			if (i != 0)
				A[i][Dimension - 1] += pow(x[j], i + Dimension - 1);
			B[i] += y[j] * pow(x[j], i);
		}

	}

	for (int i = 1; i < Dimension; i++)
		for (int j = 0; j < Dimension - 1; j++)
			A[i][j] = A[i - 1][j + 1];

	GaussSeidelSolution(NORM);
}

double LinearSystemOfLSM::solve(double x) {
	double result = 0;
	for (int j = 0; j < Dimension; j++) {
		result += X[j] * pow(x, j);
	}

	return result;
}

std::vector<long double> LinearSystemOfLSM::GaussSeidelSolution(double norm) {
	std::vector<long double> previousSol(Dimension);
	double d = 0.0;

	do
	{
		previousSol = X;

		for (int i = 0; i < Dimension; i++)
		{
			double long sum = 0;
			for (int j = 0; j < i; j++)
				sum -= (A[i][j] * X[j]);
			for (int j = i + 1; j < Dimension; j++)
				sum -= (A[i][j] * previousSol[j]);
			X[i] = (B[i] + sum) / A[i][i];
		}
		d = fabs(X[0] - previousSol[0]);
		for (int i = 0; i < Dimension; i++) {
			if (fabs(X[i] - previousSol[i]) > d)
				d = fabs(X[i] - previousSol[i]);
		}
	} while (d > norm);
	return X;
}
