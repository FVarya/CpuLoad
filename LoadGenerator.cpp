//#include "LoadGenerator.h"
#include "LSM.h"

LoadGenerator::LoadGenerator() {
	const int NUMBER_OF_CORE = 0x1;
	cpulevel_t level = CPU_LEVEL_ROOT;
	cpuwhich_t which = CPU_WHICH_PID;
	id_t id = -1;
	cpuset_t myset = CPUSET_T_INITIALIZER(NUMBER_OF_CORE);
	
	if(cpuset_setaffinity(CPU_LEVEL_WHICH, which, id, sizeof(myset), &myset) == -1){
		std::cout << "Core error" << std::endl;
	}


	std::vector<long> sleepTime(NUM_OF_POINTS_FOR_POLYNOME);// {1, 120, 300};
	std::vector<double> cpuUsage(sleepTime.size());

	int minTime = timeOfMaxLoad();
	for (int i = 0, j = minTime; i < sleepTime.size(); i++, j += minTime * i * 4) {
		sleepTime[i] = j;
		cpuUsage[i] = getCurrentLoad(sleepTime[i]);
		std::cout << "("  << cpuUsage[i] << "; " << sleepTime[i] << ")," << std::endl;


	}

	LinearSystemOfLSM approx(cpuUsage, sleepTime, DEGREE + 1);
	coeff = approx.GaussSeidelSolution(NORM);
	for (int i = 0; i < coeff.size(); i++) {
		std::cout << "coeff " << i << " " << coeff[i] << std::endl;
	}
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
		std::this_thread::sleep_for(nanoseconds(sleepTime));
	}
	return running_total;
}

double LoadGenerator::getCurrentLoad(int sleepTime) {
	cpuDump d = getCpuDump();
	startLoad(sleepTime);
	int loadTime = sleepTime * 2 < 1300000000 ? 1300000000 : sleepTime * 2;
	std::this_thread::sleep_for(nanoseconds(loadTime));
	cpuDump l = getCpuDump();
	stopLoad();
	return 100.0*(l.busy - d.busy) / (l.work - d.work);
}


int LoadGenerator::timeOfMaxLoad() {
	double currentLoad = 0.0;
	int sleepTime = START_SLEEP_TIME;
	for (; currentLoad < 90.0 &&  sleepTime > 1; sleepTime /= 10) {
		currentLoad = getCurrentLoad(sleepTime);
		std::cout << "curret load: " << currentLoad;
		std::cout << "; curret time " << sleepTime << std::endl;		
	}
	
	return sleepTime == 1 ? sleepTime : sleepTime * 10;
}

cpuDump LoadGenerator::getCpuDump() {          //300 mcrs ~ 216 - 289
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
	int sleepTime = this->coeff[0] + this->coeff[1] * load + this->coeff[2] * pow(load, 2) + this->coeff[3] * pow(load, 3);
	std::cout << "sleepTime: " << sleepTime << std::endl;
	if (sleepTime < 0) {
		throw std::string("Invalid sleep time");
	}
	//startLoad(sleepTime);
}
