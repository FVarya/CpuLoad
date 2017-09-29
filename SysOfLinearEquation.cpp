#include "LoadGenerator.h"

LoadGenerator::SysOfLinearEquation::SysOfLinearEquation(std::vector<double> x, std::vector<int> y) {
	A.resize(x.size());
	B.resize(x.size());
	X.resize(x.size());

	for (int i = 0; i < x.size(); i++) {
		A[i].resize(x.size(), 1);
		B[i] = y[i];
		for (int j = 1; j < x.size(); j++) {
			A[i][j] = pow(x[i], j);
		}
	}
}

void LoadGenerator::SysOfLinearEquation::inverse() {
	double determinant;
	for (int i = 0; i < 3; i++)
		determinant += (A[0][i] * (A[1][(i + 1) % 3] * A[2][(i + 2) % 3] -
			A[1][(i + 2) % 3] * A[2][(i + 1) % 3]));

	std::vector<std::vector<double>> mat;
	mat.resize(3);
	for (int i = 0; i < 3; i++) {
		mat[i].resize(3);
		for (int j = 0; j < 3; j++) {
			mat[i][j] = ((A[(j + 1) % 3][(i + 1) % 3] * A[(j + 2) % 3][(i + 2) % 3]) -
				(A[(j + 1) % 3][(i + 2) % 3] * A[(j + 2) % 3][(i + 1) % 3])) / determinant;
		}
	}

	A = mat;
}

std::vector<double> LoadGenerator::SysOfLinearEquation::solve() {
	inverse();
	for (int i = 0; i < 3; i++) {
		double sumInLine = 0;
		for (int j = 0; j < 3; j++) {
			sumInLine += A[i][j] * B[j];
		}
		X[i] = sumInLine;
		std::cout << X[i] << std::endl;
	}
	return X;
}