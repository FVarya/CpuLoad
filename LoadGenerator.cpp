#include "LoadGenerator.h"

LoadGenerator::LoadGenerator() {
	std::vector<int> sleepTime(NUM_OF_POINTS_FOR_POLYNOME);// {1, 120, 300};
	std::vector<double> cpuUsage(sleepTime.size());

	for (int i = 0, j = 1; i < sleepTime.size(); i++, j += STEP_OF_SLEEP_TIME) {
		sleepTime[i] = j;
		cpuDump d = getCurrentLoad();
		startLoad(sleepTime[i] * 1000);
		std::this_thread::sleep_for(milliseconds(TIME_OF_LOAD));
		cpuDump l = getCurrentLoad();
		stopLoad();
		cpuUsage[i] = 100.0*(l.busy - d.busy) / (l.work - d.work);
		std::cout << cpuUsage[i] << " " << sleepTime[i] << std::endl;

	}

	SysOfLinearEquation sys(cpuUsage, sleepTime);
	this->coeff = sys.solve();
}

LoadGenerator::~LoadGenerator() {
	if (this->thr.joinable()) {
		stopLoad();
	}
}

int LoadGenerator::generateLoad(int sleepTime) {
	int running_total = 2384;
	for (; ; ) {
		for (int i = 0; i < NUM_OF_CYCLES_IN_LOAD; i++) {
			running_total = 37 * running_total + i;
		}
		if (this->closeThread) {
			break;
		}
		std::this_thread::sleep_for(microseconds(sleepTime));
	}
	return running_total;
}

cpuDump LoadGenerator::getCurrentLoad() {          //300 mcrs ~ 216 - 289
	cpuDump curCpu;

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

void LoadGenerator::stopLoad() {
	this->closeThread = true;
	this->thr.join();
}

void LoadGenerator::startLoad(int sleepTime) {
	this->closeThread = false;
	this->thr = std::thread(&LoadGenerator::generateLoad, this, sleepTime);
}

void LoadGenerator::setLoad(double load) {
	int sleepTime = this->coeff[0] + this->coeff[1] * load + this->coeff[2] * pow(load, 2);
	std::cout << "sleepTime: " << sleepTime << std::endl;
	if (sleepTime < 0) {
		throw std::string("Invalid sleep time");
	}
	startLoad(sleepTime * 1000);
}
