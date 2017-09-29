

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


struct memory {
	unsigned long long busy = 0;
	unsigned long long work = 0;
	milliseconds ms;
};

struct interpolyaciya {
	int interval[2];
	double coefA;
	double coefB;
};


class LoadGenerator {
private:
	/*interpolyaciya coef[4];
	double coefA;
	double coefB;
	double coefC;*/
	std::vector<double> coeff;
	std::thread thr;
	bool closeThread = false;

	int generateLoad(int sleepTime) {
		int running_total = 2384;
		for (; ; ) {
			for (int i = 0; i < 10000000; i++) {
				running_total = 37 * running_total + i;
			}
			if (this->closeThread) {
				break;
			}
			std::this_thread::sleep_for(microseconds(sleepTime));
		}
		return running_total;
	}

	memory getCurrentLoad() {          //300 mcrs ~ 216 - 289
		memory curCpu;

		kvm_t *kd;
		kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm");
		if (kd) {
			struct pcpu *cpu = new pcpu;
			cpu = (struct pcpu *) kvm_getpcpu(kd, 0);
			for (int i = 0; i < CPUSTATES; i++) {
				if (i < 3) {
					curCpu.busy += cpu->pc_cp_time[i];
				}
				curCpu.work += cpu->pc_cp_time[i];
			}
			delete cpu;
		}


		kvm_close(kd);
		return curCpu;
	}

	void stopLoad() {
		this->closeThread = true;
		this->thr.join();
	}

	void startLoad(int sleepTime) {
		this->closeThread = false;
		this->thr = std::thread(&LoadGenerator::generateLoad, this, sleepTime);
	}

	class SysOfLinearEquation {
	private:
		std::vector<std::vector<double>> A;
		std::vector<double> B;
		std::vector<double> X;

		void inverse() {
			double determinant;
			for (int i = 0; i < 3; i++)
				determinant += (A[0][i] * (A[1][(i + 1) % 3] * A[2][(i + 2) % 3] - 
					A[1][(i + 2) % 3] * A[2][(i + 1) % 3]));

			std::cout << determinant << std::endl;
			std::vector<std::vector<double>> mat;
			mat.resize(3);
			for (int i = 0; i < 3; i++) {
				mat[i].resize(3);
				for (int j = 0; j < 3; j++) {
					mat[i][j] = ((A[(j + 1) % 3][(i + 1) % 3] * A[(j + 2) % 3][(i + 2) % 3]) -
						(A[(j + 1) % 3][(i + 2) % 3] * A[(j + 2) % 3][(i + 1) % 3]))/ determinant;
				}
			}

			A = mat;
		}
	public:
		SysOfLinearEquation(std::vector<double> x, std::vector<int> y) {
			A.resize(x.size());
			B.resize(x.size());
			X.resize(x.size());

			for (int i = 0; i < x.size(); i++) {
				A[i].resize(x.size(), 1);
				B[i] = y[i];
				for (int j = 1; j < x.size(); j++) {
					A[i][j] = pow(x[i], j);
					//std::cout << A[i][j] << " ";
				}
				//std::cout << std::endl;
			}
		}

		std::vector<double> solve() {
			inverse();
			for (int i = 0; i < 3; i++) {
				double sumInLine = 0;
				for (int j = 0; j < 3; j++) {
					sumInLine += A[i][j] * B[j];
					//std::cout << A[i][j] << "  ";

				}
				X[i] = sumInLine;
				std::cout << X[i] <<  std::endl;
			}
			return X;
		}
	};

public:

	~LoadGenerator() {
		if (this->thr.joinable()) {
			stopLoad();
		}
	}


	LoadGenerator() {
		/*cpu_set_t my_set;
		CPU_ZERO(&my_set);
		CPU_SET(0, &my_set);
		sched_setaffinity(0, sizeof(cpu_set_t), &my_set);*/
		std::vector<int> sleepTime { 1, 120, 300};
		std::vector<double> cpuUsage(3);
		
		for(int i = 0; i < sleepTime.size(); i++){
			memory d = getCurrentLoad();
			startLoad(sleepTime[i]*1000);
			std::this_thread::sleep_for(milliseconds(1500));
			memory l = getCurrentLoad();
			stopLoad();	
			cpuUsage[i] = 100.0*(l.busy - d.busy) / (l.work - d.work);
			std::cout << cpuUsage[i] << " " << sleepTime[i] << std::endl;

		}

		SysOfLinearEquation sys(cpuUsage, sleepTime);
		this->coeff = sys.solve();
		std::cout << "cpuUsage " << this->coeff[0] + this->coeff[1] * 70 + this->coeff[2] * pow(70, 2) << std::endl;
	}


	void setLoad(double load) {
		startLoad(this->coeff[0] + this->coeff[1] * load + this->coeff[2] * pow(load, 2));
	}
};

int main() {
	LoadGenerator ld;
	//ld.setLoad(30.0);
	//ld.stopLoad();
	std::cout << "stop" << std::endl;
}
