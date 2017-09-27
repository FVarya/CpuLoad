#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <sched.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/pcpu.h>
#include <kvm.h>


#define AVERAGE_ACCURACY 5

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
	interpolyaciya coef[4];
	double coefA;
	double coefB;
	double coefC;
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

	int calcNumOfMeasures()
	{
		double previousAccuracy;
		double accuracy = -100.0;
		int numOfMeasures = 10;

		startLoad(80000);

		for (; abs(accuracy - previousAccuracy) < AVERAGE_ACCURACY, numOfMeasures < 100; numOfMeasures += 10) {
			previousAccuracy = accuracy;
			double sumOfMeasures = 0.0;
			for (int i = 0; i < numOfMeasures; i++) {
				memory d = getCurrentLoad();
				std::this_thread::sleep_for(milliseconds(150));
				memory l = getCurrentLoad();
				std::this_thread::sleep_for(milliseconds(150));
				std::cout << 100.0*(l.busy - d.busy) / (l.work - d.work) << " ";
				sumOfMeasures += 100.0*(l.busy - d.busy) / (l.work - d.work);
			}
			accuracy = sumOfMeasures / numOfMeasures;
			std::cout << "accuracy: " << accuracy << std::endl;
		}
		stopLoad();
		return numOfMeasures;
	}

public:
	void stopLoad() {
		this->closeThread = true;
		this->thr.join();
	}

	void startLoad(int sleepTime) {
		this->thr = std::thread(&LoadGenerator::generateLoad, this, sleepTime);
	}

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
		int sleepTime[4] = { 6000, 45000, 80000, 12000 };
		memory loads[5];
		double cpuUsage[4];

		calcNumOfMeasures();
		/*
		auto start = steady_clock::now();
		int i = 0;
		this->thr = std::thread(&LoadGenerator::generateLoad, this, 6000, 100000);
		while (duration_cast<milliseconds>(steady_clock::now() - start).count() < 10000) {
			auto start1 = steady_clock::now();
			memory d = getCurrentLoad();
			std::this_thread::sleep_for(milliseconds(150));
			memory l = getCurrentLoad();
			std::this_thread::sleep_for(milliseconds(150));
			i += 15;
			std::cout << 100.0*(l.busy - d.busy) / (l.work - d.work) << " " << std::endl;

		}
		stopLoad();
		/*
		loads[0] = getCurrentLoad();

		for(int i = 0; i < 4; i++){
			int t = (i > 1) ? 1000000 : 100000;
			generateLoad(sleepTime[i], t);
			loads[i + 1] = getCurrentLoad();
			cpuUsage[i] = 100.0 * (loads[i+1].busy - loads[i].busy) / (loads[i+1].work - loads[i].work);
			std::cout << cpuUsage[i] << std::endl;
		}*/
	}


	void setLoad(double load, int time) {
		for (int i = 0; i < 4; i++) {
			if (load >= this->coef[i].interval[0] && load <= this->coef[i].interval[1]) {
				this->thr = std::thread(&LoadGenerator::generateLoad, this, (int)(this->coef[i].coefA*load + this->coef[i].coefB), time);
				break;
			}
		}
	}
};

int main() {
	LoadGenerator ld;
	//ld.setLoad(30.0, 10000000);
	//ld.stopLoad();
	std::cout << "stop" << std::endl;
}
