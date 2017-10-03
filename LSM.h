#include "LoadGenerator.h"

class LinearSystemOfLSM {
private:
	std::vector<std::vector<long double>> A;
	std::vector<long double> X;
	std::vector<long double> B;
	int Dimension;

public:
	LinearSystemOfLSM(std::vector<double> x, std::vector<long> y, int dimension);

	std::vector<long double> GaussSeidelSolution(double norm);
		//double solve(double x);

};
