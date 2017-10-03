//#include "LoadGenerator.h"

class LinearSystemOfLSM {
private:
	std::vector<std::vector<long double>> A;
	std::vector<long double> X;
	std::vector<long double> B;
	int Dimension;

	std::vector<long double> GaussSeidelSolution(double norm);
public:
	LinearSystemOfLSM(std::vector<double> x, std::vector<double> y, int dimension);

	double solve(double x);

};
