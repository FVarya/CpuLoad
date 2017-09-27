#include<cmath>
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

	int generateLoad(int sleepTime, int mcrTime) {
		std::cout << "cycle " << mcrTime / sleepTime << std::endl; 
		int running_total = 2384;
		for(int j = 0; j < mcrTime / sleepTime; j++){
			for (int i = 0; i < 10000000; i++) {
				//for(int j = 0; j < 10; j++){
					running_total = 37 * running_total + i;
				//}
			}
			if(this->closeThread){
				break;
			}
			std::this_thread::sleep_for(microseconds(sleepTime));
		}
		return running_total;
	}

	memory getCurrentLoad() {          //300 mcrs ~ 216 - 289
		memory curCpu;
		
		kvm_t *kd;
		kd = kvm_open(NULL,NULL, NULL, O_RDONLY, "kvm");
		if(kd){
			struct pcpu *cpu = new pcpu;
			cpu = (struct pcpu *) kvm_getpcpu(kd,0);
			for(int i = 0; i < CPUSTATES; i++){
				if( i < 3){
					curCpu.busy += cpu->pc_cp_time[i];
				}
				curCpu.work += cpu->pc_cp_time[i];
			}
			delete cpu;
		}
		kvm_close(kd);
		//std::cout << curCpu.busy << "   " << curCpu.work << std::endl;
		return curCpu;
		/*	
		std::string stdLine;
		int cpu;
		unsigned int  param[7];
		memory curCpu;
                
                system("vmstat -P | head -n 3 | tail -n 1 > txt");
		std::ifstream stat("txt", std::ios_base::in);
		if (!stat.is_open())
			return curCpu;

		curCpu.ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

		for(int i = 0; i < 16; i++){
			stat >> cpu;
			//std::cout << cpu << std::endl ;
		}

		for (int i = 0; i < 3; i++) {
			stat >> param[i];
			//std::cout << param[i] << " " ;
			curCpu.work += param[i];
			if (i < 2) {
				curCpu.busy += param[i];
			}
		}
		//std::cout << curCpu.busy << " " << curCpu.work << std::endl;
		return curCpu;*/
	}

public:
	void stopLoad(){
		this->closeThread = true;	
		this->thr.join();
	}
		
	LoadGenerator() {
		/*cpu_set_t my_set;        
		CPU_ZERO(&my_set);      
		CPU_SET(0, &my_set);   
		sched_setaffinity(0, sizeof(cpu_set_t), &my_set);*/
		//int x1 = 10000, x2 = 45000, x3 = 80000, x4 = 500000;
		int sleepTime[4] = {6000, 45000, 80000, 500000};
		memory loads[5];
		double cpuUsage[4];
		
		//getCurrentLoad();
		
		auto start = steady_clock::now();
		int i = 0;
		this->thr = std::thread(&LoadGenerator::generateLoad,this, 6000, 100000);	
		while(duration_cast<milliseconds>(steady_clock::now() - start).count() < 10000){
			auto start1 = steady_clock::now();
			memory d = getCurrentLoad();
			std::this_thread::sleep_for(milliseconds(150));
			memory l = getCurrentLoad();	
			std::this_thread::sleep_for(milliseconds(150));		
			i+=15;
			std::cout << 100.0*(l.busy-d.busy)/(l.work - d.work) << " "  << std::endl;
			
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
		}

		for(int i = 0; i < 3; i++){
			this->coef[i].interval[0] = cpuUsage[i+1];
			this->coef[i].interval[1] = cpuUsage[i];
			this->coef[i].coefA = (sleepTime[i+1] - sleepTime[i])/(cpuUsage[i+1] - cpuUsage[i]);
			this->coef[i].coefB = sleepTime[i] - this->coef[i].coefA*cpuUsage[i];
			std::cout << this->coef[i].interval[0] << " " << this->coef[i].interval[1] << " " << this->coef[i].coefA << " " << this->coef[i].coefB << std::endl;
		}*/

		/*
		this->coefC = (sleepTime[3]*cpuUsage[3] - sleepTime[1]*cpuUsage[1] + ((sleepTime[1] - sleepTime[3])*(sleepTime[1]*cpuUsage[1] - sleepTime[2]*cpuUsage[2]))/(sleepTime[2] - sleepTime[1]))/(cpuUsage[3] - cpuUsage[1] + ((cpuUsage[2] - cpuUsage[1])*(sleepTime[1] - sleepTime[3]))/(sleepTime[2] - sleepTime[1]) );
		this->coefB = (sleepTime[1]*cpuUsage[1] - sleepTime[2]*cpuUsage[2] + this->coefC*(cpuUsage[2] - cpuUsage[1]))/(sleepTime[2] - sleepTime[1]);
		this->coefA = (cpuUsage[1] + this->coefB)*(sleepTime[1] - this->coefC);


		std::cout << this->coefA << "   " << this->coefB << "    " << this->coefC << std::endl;
		std::cout << 	this->coefA/(70 + this->coefB) + this->coefC <<  std::endl;*/
	}


	void setLoad(double load, int time) {
		for(int i = 0; i < 4; i++){
			if(load >= this->coef[i].interval[0] && load <= this->coef[i].interval[1]){
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
