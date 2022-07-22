#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <climits>
#include <string>
#include <iostream>
#include <fstream>
#include <deque>

using namespace std;

int curr_track = 0;
bool direction = true;

struct Request {
  int timestep;
  int track;
  int io_id;
  int start_time;
  int end_time;
};

deque <Request*> dqRequest;
deque <Request*> queIO;
deque <Request*> queAdd;
deque <Request*> queAct;

class Scheduler {
public:
  virtual void AddRequest(Request* req) = 0;
  virtual Request* SetRequest() = 0;
};

class FIFO : public Scheduler {
public:
  void AddRequest(Request* req) {
    queIO.push_back(req);
  }

  Request* SetRequest() {
    if (queIO.empty()) return NULL;
    Request* ready_request = queIO.front();
    queIO.pop_front();
    return ready_request;
  }
};

class SSTF : public Scheduler {
public:
  void AddRequest(Request* req) {
    queIO.push_back(req);
  }

  Request* SetRequest() {
    int short_time = INT_MAX;
    if (queIO.empty()) return NULL;
    deque <Request*> :: iterator next_req;
    for (deque<Request*> :: iterator it = queIO.begin(); it != queIO.end(); it++) {
      if (abs((*it)->track - curr_track) < short_time) {
        short_time = abs((*it)->track - curr_track);
        next_req = it;
      }
    }
    Request* ready_request = *next_req;
    queIO.erase(next_req);
    return ready_request;
  }
};

class LOOK : public Scheduler {
public:
  void AddRequest(Request* req) {
    queIO.push_back(req);
  }

  Request* SetRequest() {
    int near = INT_MAX;
    bool change_flag = false;
    deque <Request*> :: iterator next_req;
    if (queIO.empty()) return NULL;
    if (direction==true) {
      for (deque<Request*> :: iterator it = queIO.begin(); it != queIO.end(); it++) {
        if ((*it)->track >= curr_track && (*it)->track - curr_track < near) {
            near = (*it)->track - curr_track;
            next_req = it;
            change_flag  = true;
        }
      }
    }
    else {
      for (deque<Request*> :: iterator it = queIO.begin(); it != queIO.end(); it++) {
        if ((*it)->track <= curr_track && curr_track - (*it)->track < near) {
            near = curr_track - (*it)->track;
            next_req = it;
            change_flag  = true;
        }
      }
    }
    if (!change_flag ) {
      direction = !direction;
      if (direction==true) {
        for (deque<Request*> :: iterator it = queIO.begin(); it != queIO.end(); it++) {
          if ((*it)->track >= curr_track && (*it)->track - curr_track < near) {
              near = (*it)->track - curr_track;
              next_req = it;
              change_flag  = true;
          }
        }
      }
      else {
        for (deque<Request*> :: iterator it = queIO.begin(); it != queIO.end(); it++) {
          if ((*it)->track <= curr_track && curr_track - (*it)->track < near) {
              near = curr_track - (*it)->track;
              next_req = it;
              change_flag  = true;
          }
        }
      }
    }
    Request* ready_request = *next_req;
    queIO.erase(next_req);
    return ready_request;
  }
};

class CLOOK : public Scheduler {
public:
  void AddRequest(Request* req) {
    queIO.push_back(req);
  }

  Request* SetRequest() {
    int near = INT_MAX;
    bool change_flag = false;
    Request* ready_request= NULL;
    deque <Request*> :: iterator next_req;
    if (queIO.empty()) return NULL;
    for (deque<Request*> :: iterator it = queIO.begin(); it != queIO.end(); it++) {
      if ((*it)->track >= curr_track && (*it)->track - curr_track < near) {
        near = (*it)->track - curr_track;
        next_req = it;
        change_flag = true;
      }
    }
    if (!change_flag) {
      for (deque<Request*> :: iterator it = queIO.begin(); it != queIO.end(); it++) {
        if ((*it)->track <= curr_track && abs((*it)->track) < near) {
          near = abs((*it)->track);
          next_req = it;
          change_flag = true;
        }
      }
    }
    ready_request = *next_req;
    queIO.erase(next_req);
    return ready_request;
  }  
};

class FLOOK : public Scheduler {
public:
  void AddRequest(Request* req) {
    queIO.push_back(req);
    queAdd.push_back(req);
  }

  Request* SetRequest() {
    int near = INT_MAX;
    bool change_flag = false;
    deque<Request*> :: iterator next_req;
    if (queIO.empty()) return NULL;
    if (queAct.empty()) {
      deque<Request*> temp = queAct;
      queAct = queAdd;
      queAdd = temp;
    }
    if (queAct.empty()) return NULL;
    if (direction == true) {
      for (deque<Request*> :: iterator it = queAct.begin(); it != queAct.end(); it++) {
        if ((*it)->track >= curr_track && (*it)->track - curr_track < near) {
          near = (*it)->track - curr_track;
          next_req = it;
          change_flag = true;
        }
      }
    }
    else {
      for (deque<Request*> :: iterator it = queAct.begin(); it != queAct.end(); it++) {
        if (curr_track >= (*it)->track && curr_track - (*it)->track < near) {
          near = curr_track - (*it)->track;
          next_req =it;
          change_flag = true;
        }
      }
    }
    if (!change_flag) {
      direction = !direction;
      if (direction == true) {
        for (deque<Request*> :: iterator it = queAct.begin(); it != queAct.end(); it++) {
          if ((*it)->track >= curr_track && (*it)->track - curr_track < near) {
            near = (*it)->track - curr_track;
            next_req = it;
            change_flag = true;
          }
        }
      }
      else {
        for (deque<Request*> :: iterator it = queAct.begin(); it != queAct.end(); it++) {
          if (curr_track >= (*it)->track && curr_track - (*it)->track < near) {
            near = curr_track - (*it)->track;
            next_req =it;
            change_flag = true;
          }
        }
      }
    }
    Request* ready_request = *next_req;
    queAct.erase(next_req);
    queIO.pop_front();
    return ready_request;
  }
};


Scheduler* scheduler;

// ========== READ FILE ==========
void ReadFile(string IO_file) {
  fstream file (IO_file.c_str());
  string line;
  Request* io_request;
  int num = 0;
  if (file.is_open()) {
    while (getline(file, line)&&!file.eof()){
      if (line.empty()||line[0]=='#') continue;
      io_request = new Request();
      io_request->io_id = num++;
      sscanf(line.c_str(), "%d %d", &io_request->timestep, &io_request->track);
      io_request->start_time = 0;
      io_request->end_time = 0;
      dqRequest.push_back(io_request);
    }
  }
  file.close();
}

  void simulation() {
    int index = 0;
    int tot_movement = 0;
    int total_time = 0;
    int max_waittime = 0;
    double avg_turnaround = 0.0;
    double avg_waittime = 0.0;
    deque<Request*>::iterator new_request = dqRequest.begin();
    Request* act_request = NULL;

    while (!queIO.empty() || index < dqRequest.size() || act_request != NULL) {
      if (index < dqRequest.size() && dqRequest[index]->timestep == total_time) {
        scheduler->AddRequest(dqRequest[index]);
        index++;
      }
      if (act_request != NULL) {
        if (act_request->track == curr_track) {
          act_request->end_time = total_time;
          act_request = NULL;
        }
        else if (act_request->track > curr_track) {
          tot_movement++;
          direction = true;
          curr_track++;
        }
        else if (act_request->track < curr_track) {
          tot_movement++;
          direction = false;
          curr_track--;
        }
      }
      if (act_request == NULL && !queIO.empty()) {
        act_request = scheduler->SetRequest();
        act_request->start_time = total_time;
        continue;
      }
      total_time++;
    }
    for (deque <Request*> :: iterator it = dqRequest.begin(); it != dqRequest.end(); it++) {
      avg_turnaround += (*it)->end_time - (*it)->timestep;
      avg_waittime += (*it)->start_time - (*it)->timestep;
      if ((*it)->start_time-(*it)->timestep > max_waittime) max_waittime = (*it)->start_time - (*it)->timestep;
      printf ("%5d: %5d %5d %5d\n", (*it)->io_id, (*it)->timestep, (*it)->start_time, (*it)->end_time);
    }
    avg_turnaround = avg_turnaround / dqRequest.size();
    avg_waittime = avg_waittime / dqRequest.size();
    printf ("SUM: %d %d %.2lf %.2lf %d\n", --total_time, tot_movement, avg_turnaround, avg_waittime, max_waittime);
  }

int main(int argc, char* argv[]) {
  char c;
  char algo;
  setvbuf(stdout, NULL, _IONBF, 0);

  while ((c = getopt(argc, argv, "s:vqf")) != -1) {
    switch (c) {
      case 's':
        if (!optarg || (*optarg != 'i' && *optarg != 'j' && 
                        *optarg != 's' && *optarg != 'c' && 
                        *optarg != 'f')) return -1;
        algo = *optarg;
        switch (algo) {
          case 'i': scheduler = new FIFO(); break;
          case 'j': scheduler = new SSTF(); break;
          case 's': scheduler = new LOOK(); break;
          case 'c': scheduler = new CLOOK(); break;
          case 'f': scheduler = new FLOOK(); break;
          default: scheduler = new FIFO(); break;
        }
        break;
      case '?': return -1;
      default: printf("Invalid Arguments!"); return -1;
    }
  }
  ReadFile(argv[argc-1]);
  simulation();
  return 0;
}