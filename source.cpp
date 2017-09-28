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


#define AVERAGE_ACCURACY 2.0

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
		double previousAccuracy = 0.0;
		double accuracy = 100.0;
		int numOfMeasures = 5;

		startLoad(80000);
		

		for (; abs(accuracy - previousAccuracy) >  AVERAGE_ACCURACY &&  numOfMeasures < 50; numOfMeasures += 5) {
			previousAccuracy = accuracy;
			accuracy = measure(numOfMeasures);
			std::cout << "abs " << abs(accuracy - previousAccuracy) << std::endl;
		}
		stopLoad();
		return numOfMeasures - 5;
	}

	double measure(int numOfMeasures)
	{
		double sumOfMeasures = 0.0;
		for (int i = 0; i < numOfMeasures; i++) {
			memory d = getCurrentLoad();
			std::this_thread::sleep_for(milliseconds(150));
			memory l = getCurrentLoad();
			std::this_thread::sleep_for(milliseconds(150));
			sumOfMeasures += 100.0*(l.busy - d.busy) / (l.work - d.work);
		}
		std::cout << "meas " << sumOfMeasures / numOfMeasures << std::endl;

		return sumOfMeasures / numOfMeasures;
	}

	void stopLoad() {
		this->closeThread = true;
		this->thr.join();
	}

	void startLoad(int sleepTime) {
		this->closeThread = false;
		this->thr = std::thread(&LoadGenerator::generateLoad, this, sleepTime);
	}

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
		int sleepTime[4] = { 6000, 45000, 80000, 300000 };
		memory loads[5];
		double cpuUsage[4];

		int numOfMeasures = calcNumOfMeasures();
		
		for(int i = 0; i < 4; i++){
			startLoad(sleepTime[i]);
			cpuUsage[i] = measure(numOfMeasures);
			stopLoad();	
			std::cout << cpuUsage[i] << " " << sleepTime[i] << std::endl;
		}
	}


	void setLoad(double load, int time) {
		for (int i = 0; i < 4; i++) {
			if (load >= this->coef[i].interval[0] && load <= this->coef[i].interval[1]) {
				this->thr = std::thread(&LoadGenerator::generateLoad, this, (int)(this->coef[i].coefA*load + this->coef[i].coefB));
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
