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


struct Instructions {
   int timeStamp;   //First int
   int track;       //Second int
   int num;         //Num in list
};

class Scheduler{
  public:
    virtual Instructions getNextIO(vector<Instructions>& ioQueue, int& currentTime, bool direction) = 0;

    virtual bool updateQueue(vector<Instructions>& ioQueue){
      return false;
    }

    virtual Instructions addTrack(vector<Instructions>& instrList, vector<Instructions>& ioQueue, bool flag){
      Instructions tmp = instrList.front();
      instrList.erase(instrList.begin());
      ioQueue.push_back(tmp);
      return tmp;
    };
};

class FIFO: public Scheduler{
  public:
    Instructions getNextIO(vector<Instructions>& ioQueue, int& currentTime, bool direction);
};

Instructions FIFO::getNextIO(vector<Instructions>&ioQueue, int& currentTime, bool direction){
  Instructions instr;
  if(!ioQueue.empty()){
      instr = ioQueue.front();
      ioQueue.erase(ioQueue.begin());
  }
  return instr;
};


class SSTF: public Scheduler{
  public:
    Instructions getNextIO(vector<Instructions>& ioQueue, int& currentTime, bool direction);
};

Instructions SSTF::getNextIO(vector<Instructions>&ioQueue, int& currentTime, bool direction){
  unsigned int dis = 0xffffffff;
  int minIndex = -1;

  for (int i=0; i<ioQueue.size(); i++){
    int tmp;
    tmp = abs(currentTime - ioQueue[i].track);
    if (tmp < dis){
        dis = tmp;
        minIndex = i;
    }
  }
  Instructions tmp;
  tmp = ioQueue[minIndex];
  ioQueue.erase(ioQueue.begin()+minIndex);
  return tmp;
};


class SCAN: public Scheduler{
  public:
    Instructions getNextIO(vector<Instructions>& ioQueue, int& currentTime, bool direction);
  protected:
    static bool sortRight(Instructions a, Instructions b);
    static bool sortLeft(Instructions a, Instructions b);
};

bool SCAN::sortRight(Instructions a, Instructions b) {
  if(a.track==b.track){
    return a.num < b.num;
  }
  else{
    return a.track < b.track;
  }
}

bool SCAN::sortLeft(Instructions a, Instructions b) {
  if(a.track==b.track){
    return a.num > b.num;
  }
  else{
    return a.track < b.track;
  }
}

Instructions SCAN::getNextIO(vector<Instructions>&ioQueue, int& currentTime, bool direction){
  //Take the closest request in the direction of travel (left or right)
  int minIndex=0;
  if (direction){
    sort(ioQueue.begin(), ioQueue.end(), sortRight);
  }
  else{
    sort(ioQueue.begin(), ioQueue.end(), sortLeft);
  }

  if (direction){
    minIndex = ioQueue.size()-1;
		for (int i=0; i<ioQueue.size(); i++){
			if (ioQueue[i].track >= currentTime){
				minIndex = i;
				break;
			}
		}
  }
  else{
    minIndex = 0;
		for (int i=ioQueue.size()-1; i>=0; i--){
			if (ioQueue[i].track <= currentTime){
				minIndex = i;
				break;
			}
		}
	}
	Instructions tmp = ioQueue[minIndex];
	ioQueue.erase(ioQueue.begin()+minIndex);
	return tmp;
};


class CSCAN: public SCAN{
  public:
    Instructions getNextIO(vector<Instructions>& ioQueue, int& currentTime, bool direction);
};

Instructions CSCAN::getNextIO(vector<Instructions>&ioQueue, int& currentTime, bool direction){
  //Sort circular left to right
  int minIndex=0;
  sort(ioQueue.begin(), ioQueue.end(), sortRight);
  for (int i=0; i<ioQueue.size(); i++){
    if (ioQueue[i].track >= currentTime){
      minIndex = i;
      break;
    }
  }
  Instructions tmp = ioQueue[minIndex];
  ioQueue.erase(ioQueue.begin()+minIndex);
  return tmp;
};


class FSCAN: public SCAN{
  private:
    vector<Instructions> instructionBuffer;
  public:
    bool updateQueue(vector<Instructions>& ioQueue);
    Instructions getNextIO(vector<Instructions>& ioQueue, int& currentTime, bool direction);
    Instructions addTrack(vector<Instructions>& instrList, vector<Instructions>& ioQueue, bool flag);
};

bool FSCAN::updateQueue(vector<Instructions>& ioQueue) {
    if (!ioQueue.empty()){
      return false;
    }
    else{
      ioQueue = instructionBuffer;
      instructionBuffer.clear();
      return true;
    }
}

Instructions FSCAN::addTrack(vector<Instructions>& instrList, vector<Instructions>& ioQueue, bool flag){
    Instructions tmp = instrList.front();
    instrList.erase(instrList.begin());

    if (!flag){
      ioQueue.push_back(tmp);
    }
    else{
      instructionBuffer.push_back(tmp);
    }
    return tmp;
};

Instructions FSCAN::getNextIO(vector<Instructions>&ioQueue, int& currentTime, bool direction){
  //This is exactly same as SCAN
  int minIndex=0;
  if (direction){
    sort(ioQueue.begin(), ioQueue.end(), sortRight);
  }
  else{
    sort(ioQueue.begin(), ioQueue.end(), sortLeft);
  }

  if (direction){
    minIndex = ioQueue.size()-1;
		for (int i=0; i<ioQueue.size(); i++){
			if (ioQueue[i].track >= currentTime){
				minIndex = i;
				break;
			}
		}
  }
  else{
    minIndex = 0;
		for (int i=ioQueue.size()-1; i>=0; i--){
			if (ioQueue[i].track <= currentTime){
				minIndex = i;
				break;
			}
		}
	}
	Instructions tmp = ioQueue[minIndex];
	ioQueue.erase(ioQueue.begin()+minIndex);
	return tmp;
};



bool setDirection(int currentTime, int previousTime);
vector<Instructions> instrList;
vector<Instructions> ioQueue;
Scheduler *sch;

//Main Program
int main(int arg, char* argv[]){

  if (arg < 3)
	{
		exit (EXIT_FAILURE);
	}

  //Read Input file
  ifstream myReadFile;
  myReadFile.open(argv[2]);
  string line;
  Instructions instr;
  int timeStamp,track;
  int num=0;

  int instrSize=0;
  int count=0;
  int arriveTime=1;
  int currentTime=0;
  int previousTime=0;

  int waitTime=0;
  int turnTime=0;
  bool flag=false;    //when flag is true, add track to ioQueue
  bool direction;

  int total_time=0;
  int tot_movement=0;
  int max_waittime=0;
  int total_waittime=0;
  int total_turnaround=0;
  double avg_turnaround=0;
  double avg_waittime=0;

  while(getline(myReadFile, line)){
    if (line[0] == '#'){
      continue;
    }
    istringstream token(line);
    token >> timeStamp >> track;
    Instructions tmp;
    tmp.timeStamp = timeStamp;
    tmp.track = track;
    tmp.num = num;
    instrList.push_back(tmp);
    num++;
  }
  instrSize = instrList.size();
  myReadFile.close();


  //Assign scheduler algorithm
  if(argv[1][2] == 'i') {
    sch = new FIFO();
	}
	else if (argv[1][2] == 'j'){
		sch = new SSTF();
	}
	else if (argv[1][2] == 's'){
		sch = new SCAN();
	}
	else if (argv[1][2] == 'c'){
		sch = new CSCAN();
	}
	else if (argv[1][2] == 'f'){
		sch = new FSCAN();
	}


  

  //Simulation
  //cout<<"TRACE"<<endl;

  while (count != instrSize){
    //Take next track from ioQueue
    if (!flag && !ioQueue.empty()){
      direction = setDirection(currentTime,  previousTime);
      instr = sch->getNextIO(ioQueue, currentTime, direction);
      tot_movement += abs(currentTime - instr.track);
      waitTime = total_time - instr.timeStamp;
      total_waittime += waitTime;
      max_waittime = max(max_waittime, waitTime);
      //cout << arriveTime << ":     " << instr.num << " issue " << instr.track<<" "<<currentTime<<endl;
      flag = true;
    }

    //Issue ready tracks into ioQueue
    if (flag){
      arriveTime = total_time + abs(currentTime - instr.track);
      while (!instrList.empty() && instrList[0].timeStamp <= arriveTime){
        Instructions tmp;
        tmp = sch->addTrack(instrList, ioQueue, flag);
        total_time = tmp.timeStamp;
        //cout<<tmp.timeStamp<< ":     " << tmp.num<<" add "<<tmp.track<<endl;
      }

      if (instr.track != currentTime){
        previousTime = currentTime;
      }

      currentTime = instr.track;
      total_time = arriveTime;
      turnTime = total_time - instr.timeStamp;
      total_turnaround += turnTime;
      //cout << arriveTime << ":     " << instr.num << " finish " << total_time-instr.timeStamp <<endl;
      count++;
      flag = false;
    }

    //ioQueue is empty
    else{
      //cout << arriveTime << ":     " << instrList[0].num << " add " << instrList[0].track <<endl;
      Instructions tmp;
      tmp = sch->addTrack(instrList, ioQueue, flag);
  		total_time = tmp.timeStamp;
    }

    if(sch->updateQueue(ioQueue)){
      previousTime = currentTime;
    }
  }

  avg_turnaround = (double)total_turnaround/instrSize;
	avg_waittime =  (double)total_waittime/instrSize;

  //Print Results
  printf("SUM: %d %d %.2lf %.2lf %d\n",total_time,tot_movement,avg_turnaround,avg_waittime,max_waittime);

}



bool setDirection(int currentTime, int previousTime){
  if (currentTime-previousTime>=0){
    return true;    //Right
  }
  else{
    return false;  //Left
  }
}
