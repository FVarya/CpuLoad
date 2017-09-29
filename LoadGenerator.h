#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/pcpu.h>
#include <kvm.h>

using namespace std::chrono;

#define NUM_OF_CYCLES_IN_LOAD 10000000
#define NUM_OF_POINTS_FOR_POLYNOME 3
#define STEP_OF_SLEEP_TIME 30
#define TIME_OF_LOAD 1500

struct cpuDump {
	unsigned long long busy = 0;
	unsigned long long work = 0;
};


class LoadGenerator {
private:
	std::vector<double> coeff;
	std::thread thr;
	bool closeThread = false;

	int generateLoad(int sleepTime);
	cpuDump getCurrentLoad();
	void startLoad(int sleepTime);

	class SysOfLinearEquation {
	private:
		std::vector<std::vector<double>> A;
		std::vector<double> B;
		std::vector<double> X;

		void inverse();
	public:
		SysOfLinearEquation(std::vector<double> x, std::vector<int> y);

		std::vector<double> solve();
	};

public:

	LoadGenerator();
	~LoadGenerator();

	void setLoad(double load);
	void stopLoad();
};