#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <map>
#include <stack>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <string>
#include <cstring>
#include <ctype.h>
using namespace std;

#define CREATED 0
#define READY 1
#define RUNNG 2
#define BLOCK 3
#define DONE 4


class Process{
	public:

		int pid;						//ID
		int AT;							//arrival time
		int TC;							//total cpu time
		int CB;							//CPU burst
		int IO;							//IO burst
		int FT;							//Finishing time
		int TT;							//turnaround time (finsihing time - AT)
		int IT;							//time in blocked state
		int CW;							//time in ready state
		int rem;						//remaining CPU time
		int staticPRIO;
		int dynamicPRIO;
		int timeInPrevState;
		int rem_CB;
		Process();
};

Process::Process(){
}

class Event{
	 public:
		 int timestamp;
		 int pid;
		 int oldstate;
		 int newstate;
		 int timeInPrevState;
		 int timeEventCreated;
		 Event();
		 Event (int ts, int id, int os, int ns, int evtCreated);
};

Event::Event(int ts, int id, int os, int ns, int evtCreated){
	timestamp = ts;
	pid = id;
	oldstate = os;
	newstate = ns;
	timeEventCreated = evtCreated;
}

Event::Event(){
}


class Scheduler{
	public:
			vector <Process> runQueue;
			virtual Process getNextProcess() = 0;									//Get the next event with smallest ID
			virtual void addProcess(Process next_process) = 0;		//Add Process to run queue
			virtual void addexProcess(Process proc) = 0;
			virtual int changePRIO(int PRIO) = 0;
};


//FCFS implementation
class FCFS: public Scheduler{
	public:
		void addProcess(Process proc);
		void addexProcess(Process proc);
		Process getNextProcess();
		int changePRIO(int PRIO);
};

void FCFS::addProcess(Process proc)
{
	runQueue.push_back(proc);
}

Process FCFS::getNextProcess(){
  Process proc;
  if(!runQueue.empty()){
      proc = runQueue.front();
      runQueue.erase(runQueue.begin());
  }
  return proc;
}

int FCFS::changePRIO(int PRIO){
	return PRIO;
}

void FCFS::addexProcess(Process proc){
}


//LCFS implementation
class LCFS: public Scheduler{
	public:
		void addProcess(Process proc);
		void addexProcess(Process proc);
		Process getNextProcess();
		int changePRIO(int PRIO);

};

void LCFS::addProcess(Process proc)
{
	runQueue.push_back(proc);
}

Process LCFS::getNextProcess(){
	Process proc;
	if (!runQueue.empty()) {
			proc = runQueue.back();
			runQueue.pop_back();
	}
	return proc;
}

int LCFS::changePRIO(int PRIO){
	return PRIO;
}

void LCFS::addexProcess(Process proc){
}




//RoundRobin implementation
class RoundRobin: public Scheduler{
	public:
		void addProcess(Process proc);
		void addexProcess(Process proc);
		Process getNextProcess();
		int changePRIO(int PRIO);
};

void RoundRobin::addProcess(Process proc)
{
	runQueue.push_back(proc);
}

Process RoundRobin::getNextProcess(){
	Process proc;
	if(!runQueue.empty()){
			proc = runQueue.front();
			runQueue.erase(runQueue.begin());
	}
	return proc;
}

int RoundRobin::changePRIO(int PRIO){
	return PRIO;
}

void RoundRobin::addexProcess(Process proc){
}




//ShortestJobFirst Scheduler
class SJF: public Scheduler{
	public:
		void addProcess(Process proc);
		void addexProcess(Process proc);
		Process getNextProcess();
		int changePRIO(int PRIO);
};

void SJF::addProcess(Process proc)
{
	runQueue.push_back(proc);
}

Process SJF::getNextProcess(){
	Process proc;
	int min = runQueue[0].rem;
  int tmp = 0;
	if(!runQueue.empty()){
		for (int i=1; i < runQueue.size(); i++){
			if (runQueue[i].rem < min) {
					min = runQueue[i].rem;
					tmp = i;
			}
		}
		proc = runQueue[tmp];
    runQueue.erase(runQueue.begin() + tmp);
	}
	return proc;
}

int SJF::changePRIO(int PRIO){
	return PRIO;
}

void SJF::addexProcess(Process proc){
}



//Priority Scheduler
class PRIO: public Scheduler{
	public:
		//Active Queue
		vector<Process> actQueue0;
		vector<Process> actQueue1;
		vector<Process> actQueue2;
		vector<Process> actQueue3;
		//Expired Queue
		vector<Process> expQueue0;
		vector<Process> expQueue1;
		vector<Process> expQueue2;
		vector<Process> expQueue3;

		void addProcess(Process proc);
		void addexProcess(Process proc);
		Process getNextProcess();
		int changePRIO(int PRIO);

};


void PRIO::addProcess(Process proc){
	if (proc.dynamicPRIO == 0){
		actQueue0.push_back(proc);
	}
	else if (proc.dynamicPRIO == 1){
		actQueue1.push_back(proc);
	}
	else if (proc.dynamicPRIO == 2){
		actQueue2.push_back(proc);
	}
	else{
		actQueue3.push_back(proc);
	}
}

void PRIO::addexProcess(Process proc){
	if (proc.dynamicPRIO == 0){
		expQueue0.push_back(proc);
	}
	else if (proc.dynamicPRIO == 1){
		expQueue1.push_back(proc);
	}
	else if (proc.dynamicPRIO == 2){
		expQueue2.push_back(proc);
	}
	else{
		expQueue3.push_back(proc);
	}
}

Process PRIO::getNextProcess(){
	Process proc;
	if (actQueue0.empty() && actQueue1.empty() && actQueue2.empty() && actQueue3.empty()){
		actQueue0.swap(expQueue0);
		actQueue1.swap(expQueue1);
		actQueue2.swap(expQueue2);
		actQueue3.swap(expQueue3);
	}

	//Get next processes by priority classes
	if (!actQueue3.empty()) {
    proc = actQueue3.front();
    actQueue3.erase(actQueue3.begin());
  }
  else if (!actQueue2.empty()) {
    proc = actQueue2.front();
		actQueue2.erase(actQueue2.begin());
  }
  else if (!actQueue1.empty()) {
    proc = actQueue1.front();
    actQueue1.erase(actQueue1.begin());
  }
  else if (!actQueue0.empty()) {
    proc = actQueue0.front();
    actQueue0.erase(actQueue0.begin());
  }
	return proc;
}

int PRIO::changePRIO(int PRIO){
	return PRIO-1;
}



Event getEvent();
vector<Process> processes;
vector<Event> events;
vector<int> randomValues;
void readRandomNumbers(char* rfile);
void insertEvent(Event createEvent);
int myRandom(int burst);
int ofs = 0;
int quantum = 10000;
int earlyProcess=0;
Scheduler *sch;

int main(int arg, char* argv[]){
	ifstream myReadFile;

	int id=0;
	int a=0,b=0,c=0,d=0;
	int current_time=0;
	int current_pid=0;
	bool call_scheduler = false;
	int random_CB=0;
	int countcount=0;
	int io_time=0;
  int lastBlock=0;
	string scheduler_type;

	if (arg < 4)
	{
		exit (EXIT_FAILURE);
	}

	//Initialise scheduler
	string tmp;
	if(argv[1][2] == 'F') {
		sch = new FCFS();
		scheduler_type = "FCFS";
		quantum = 10000;
	}
	else if (argv[1][2] == 'L'){
		sch = new LCFS();
		scheduler_type = "LCFS";
		quantum = 10000;
	}
	else if (argv[1][2] == 'S'){
		sch = new SJF();
		scheduler_type = "SJF";
		quantum = 10000;
	}
	else if (argv[1][2] == 'R'){
		sch = new RoundRobin();
		scheduler_type = "RR";
		for (int i=3; i < strlen(argv[1]); i++) {
				tmp += argv[1][i];
		}
		quantum = atoi(tmp.c_str());
	}
	else if (argv[1][2] == 'P'){
		sch = new PRIO();
		scheduler_type = "PRIO";
		for (int i=3; i < strlen(argv[1]); i++) {
				tmp += argv[1][i];
		}
		quantum = atoi(tmp.c_str());
	}


	if (argv[2] == NULL)
	{
		exit (EXIT_FAILURE);
	}

	if (argv[3] == NULL)
	{
		exit (EXIT_FAILURE);
	}

	// Read random number file
	readRandomNumbers(argv[3]);

	// Read input file
	myReadFile.open(argv[2]);
	if (myReadFile.is_open()){
      while (myReadFile >> a >> b >> c >> d){
						Process tmp;
						tmp.pid = id;
	  			  tmp.AT = a;
	          tmp.TC = b;
	          tmp.CB = c;
	          tmp.IO = d;
						tmp.staticPRIO = myRandom(4);
						tmp.dynamicPRIO = tmp.staticPRIO-1;
						tmp.rem=b;
						tmp.FT=0;
						tmp.TT=0;
						tmp.IT=0;
						tmp.CW=0;
						tmp.timeInPrevState=0;
						tmp.rem_CB=0;
						processes.push_back(tmp);
						id ++;
			}
  }
	myReadFile.close();


	//Create event for all processes in input file
	for (int i=0; i<processes.size(); i++){
		Event createEvent(processes[i].AT, processes[i].pid, CREATED, READY, processes[i].AT);
		//Insert event into queue
		insertEvent(createEvent);
	};



	//Simulation
	while(!events.empty()){
	  Event curEvent;

		//Get Event and delete the first event after queue
	  curEvent = events.front();
		events.erase(events.begin());

	  current_time = curEvent.timestamp;
		current_pid = curEvent.pid;

		//CREATED to READY (need to add a process to runQueue)
		if (curEvent.oldstate == CREATED && curEvent.newstate == READY){
			processes[current_pid].timeInPrevState = current_time;
			sch->addProcess(processes[current_pid]);
			Event evt(processes[current_pid].AT, current_pid, READY, RUNNG, processes[current_pid].AT);
			insertEvent(evt);
			//cout << current_time <<" "<<current_pid<<" "<<processes[current_pid].CW<<": CREATED -> READY" << endl;
		}

		//READY to RUNNING
		//Need to get next process from run queue
		else if (curEvent.oldstate == READY && curEvent.newstate == RUNNG){

			if (curEvent.timestamp < earlyProcess){
			  curEvent.timestamp = earlyProcess;
			  insertEvent(curEvent);
				continue;
			}

			Process proc;
			proc = sch->getNextProcess();
			current_pid = proc.pid;
			curEvent.timeEventCreated = processes[current_pid].timeInPrevState;
			processes[current_pid].CW += current_time - curEvent.timeEventCreated;

			//Select CPU Burst depending on quantum
			if(processes[current_pid].rem_CB > 0){
				random_CB = processes[current_pid].rem_CB;
			}
			else{
				random_CB = myRandom(processes[current_pid].CB);
			}
			if (random_CB > processes[current_pid].rem){
				random_CB = processes[current_pid].rem;
			}

			if (quantum < random_CB){
				// Use quantum
				//cout << current_time <<" "<<current_pid<<" "<< current_time - curEvent.timeEventCreated <<": READY -> RUNNG";
				//cout << " cb=" << random_CB << " rem=" << processes[current_pid].rem << " prio=" <<processes[current_pid].dynamicPRIO<<endl;
				processes[current_pid].rem_CB = random_CB - quantum;
				processes[current_pid].rem -= quantum;
				Event evt(current_time + quantum, current_pid, RUNNG, READY, current_time);
				insertEvent(evt);
				earlyProcess = current_time + quantum;
				//processes[current_pid].dynamicPRIO = sch->changePRIO(processes[current_pid].dynamicPRIO);
			}

			else{
				//cout << current_time <<" "<<current_pid<<" "<< current_time - curEvent.timeEventCreated <<": READY -> RUNNG";
				if (processes[current_pid].rem <= random_CB){
					//cout << " cb=" << processes[current_pid].rem << " rem=" << processes[current_pid].rem << " prio=" <<processes[current_pid].dynamicPRIO<<endl;
					processes[current_pid].rem -= random_CB;
					Event evt(current_time + random_CB, current_pid, RUNNG, DONE, current_time);
					insertEvent(evt);
					earlyProcess = current_time + random_CB;
				}
				else{
					//cout << " cb=" << random_CB << " rem=" <<processes[current_pid].rem << " prio=" <<processes[current_pid].dynamicPRIO<<endl;
					processes[current_pid].rem -= random_CB;
					processes[current_pid].rem_CB = 0;
					Event evt(current_time + random_CB, current_pid, RUNNG, BLOCK, current_time);
					insertEvent(evt);
					earlyProcess = current_time + random_CB;
					//processes[current_pid].dynamicPRIO = sch->changePRIO(processes[current_pid].dynamicPRIO);
				}
			}
		}

		//RUNNING to BLOCK
		else if (curEvent.oldstate == RUNNG && curEvent.newstate == BLOCK){
			int random_IO = myRandom(processes[current_pid].IO);

			if (current_time >= lastBlock){
          io_time += random_IO;
          lastBlock = current_time+random_IO;
      }
      else {
          if (current_time+random_IO > lastBlock){
          	io_time += current_time + random_IO - lastBlock;
          	lastBlock = current_time+random_IO;
          }
      }
			processes[current_pid].IT += random_IO;
			Event evt(current_time + random_IO, current_pid, BLOCK, READY, current_time);
			insertEvent(evt);
			//cout << current_time <<" "<<current_pid<<" "<<current_time - curEvent.timeEventCreated<<": RUNNG -> BLOCK";
			//cout << "  ib=" << random_IO <<" rem="<<processes[current_pid].rem<<endl;
		}

		//BLOCK to READY (need to add a process to runQueue)
		else if (curEvent.oldstate == BLOCK && curEvent.newstate == READY){
			processes[current_pid].dynamicPRIO = processes[current_pid].staticPRIO-1;
			processes[current_pid].timeInPrevState = current_time;
			processes[current_pid].rem_CB = 0;
			sch->addProcess(processes[current_pid]);
			Event evt(current_time, current_pid, READY, RUNNG, current_time);
			insertEvent(evt);
			//cout << current_time <<" "<<current_pid<<" "<<current_time - curEvent.timeEventCreated<<": BLOCK -> READY" << endl;
		}

		//RUNNING to READY (need to add a process to runQueue)
		else if (curEvent.oldstate == RUNNG && curEvent.newstate == READY){
			//cout << processes[current_pid].dynamicPRIO <<endl;
			if(processes[current_pid].rem_CB > 0){
				//cout << current_time <<" "<<current_pid<<" "<<current_time - curEvent.timeEventCreated<< ": RUNNG -> READY";
				//cout << "  cb=" << processes[current_pid].rem_CB << " rem=" <<processes[current_pid].rem << " prio=" <<processes[current_pid].dynamicPRIO<< endl;
			}
			processes[current_pid].dynamicPRIO = sch->changePRIO(processes[current_pid].dynamicPRIO);
			processes[current_pid].timeInPrevState = current_time;
			Event evt(current_time, current_pid, READY, RUNNG, current_time);

			if (processes[current_pid].dynamicPRIO == -1){
				processes[current_pid].dynamicPRIO = processes[current_pid].staticPRIO-1;
				sch->addexProcess(processes[current_pid]);
			}
			else{
				sch->addProcess(processes[current_pid]);
			}
			insertEvent(evt);
			//cout << processes[current_pid].rem_CB << endl;
			//processes[current_pid].rem_CB -= quantum;

		}

		//RUNNING to DONE
		else {
			processes[current_pid].FT = current_time;
			processes[current_pid].TT = current_time - processes[current_pid].AT;
			//cout << current_time <<" "<< current_pid << " "<<current_time - curEvent.timeEventCreated<<": ";
			//cout << "Done" << endl;
		}


	}

	double cpu_util = 0.0;
 	double io_util= 0.0;
	double avg_turnaround= 0.0;
	double avg_cpu= 0.0;
	double throughput = 0.0;

	//Print Summary
	cout <<scheduler_type;
	if (quantum != 10000){
		cout<<" "<<quantum<<endl;
	}
	else{
		cout<<endl;
	}

	for (int i=0; i<processes.size(); i++)
	{
		printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
		       processes[i].pid,
		       processes[i].AT, processes[i].TC, processes[i].CB, processes[i].IO, processes[i].staticPRIO,
		       processes[i].FT, // last time stamp
		       processes[i].TT,
		       processes[i].IT,
		       processes[i].CW);

		cpu_util += processes[i].TC;
		avg_turnaround += processes[i].FT - processes[i].AT;
		avg_cpu += processes[i].CW;
	}

	throughput= (double) processes.size() * 100 / (double) current_time;

	printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
	       current_time,
	       (double) cpu_util/current_time*100,
	       (double) io_time/current_time*100,
	       avg_turnaround/processes.size(),
	       avg_cpu/processes.size(),
	       throughput);
}


//Insert event into queue in order
void insertEvent(Event createEvent){
	int tmp = 0;
	while (tmp < events.size() && createEvent.timestamp >= events[tmp].timestamp){
		tmp++;
	}
	events.insert(events.begin() + tmp, createEvent);
};


//Read random number file
void readRandomNumbers(char* rfile){
	//Read random number file
	ifstream randomNumFile;
	randomNumFile.open(rfile);
	int totalNum=0;
	int tmp=0;
	int i=0;
	randomNumFile >> totalNum;
	while (randomNumFile >> tmp){
		randomValues.push_back(tmp);
	}
	randomNumFile.close();
};

//Generate random number
int myRandom(int burst)
{
		if(ofs == randomValues.size()-1)
				ofs = 0;
		int returnValue = 1 + randomValues[ofs] % burst;
		ofs++;
		return  returnValue;
};
