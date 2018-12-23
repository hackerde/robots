#ifndef _ROBOT_
#define _ROBOT_

#include <iostream>
#include <string>
#include <cmath>
#include "mpi.h"
#include "server.h"
#include <ctime>

using namespace std;

MPI_Status stat;
int zero[1] = {0};
int pos[2];

void stosleep(long msec)
{
	struct timeval tv;
	tv.tv_sec = msec/1000;
	tv.tv_usec = 1000*msec%1000000;
	select(0, NULL, NULL, NULL, &tv);
}

class Robot
{
	int pos_x, pos_y;		// position of robot on map
	int job_time;			// time to perform currently assigned job
	int job_x, job_y;		// coordinates of assigned job
	int step_x, step_y;		// steps to take to reach destination
	int server_rank;		// rank of server process
	int adjacent_pos[4];		// info of adjacent squares
	int * path;
	int path_len;
	int isClear;
	
public:

	Robot(int sr) {			// constructor with rank of server
	  server_rank = sr;
	  MPI_Send(zero, 1, MPI_INT, server_rank, INIT, MPI_COMM_WORLD);	// send INIT request
	  int pos[3];
	  MPI_Recv(pos, 3, MPI_INT, server_rank, ACK, MPI_COMM_WORLD, &stat);	// receive initial coordinates
	  pos_x = pos[0];		// set coordinates
	  pos_y = pos[1];
	}

	void getJob() {			
	// ask for a job

	  MPI_Send(zero, 1, MPI_INT, server_rank, JOB_REQUEST, MPI_COMM_WORLD);
	  int buff[4];
	  MPI_Recv(buff, 4, MPI_INT, server_rank, ACK, MPI_COMM_WORLD, &stat);		// receive job details

	  MPI_Probe(server_rank, ACK, MPI_COMM_WORLD, &stat);
	  MPI_Get_elements(&stat, MPI_INT, &path_len);
	  path = new int[path_len];
	  MPI_Recv(path, path_len, MPI_INT, server_rank, ACK, MPI_COMM_WORLD, &stat);
 
	  job_x = buff[0]; 
	  job_y = buff[1]; 
	  job_time = buff[2]; 
	  step_x = job_x - pos_x; 
	  step_y = job_y - pos_y; 
	  cout << "New job at: " << job_x << " " << job_y << " " << job_time << " | My Info: " << pos_x << " " <<  pos_y << " " << " | Distance: " << step_x << " " << step_y << endl; 
	}

	void doJob() {
	// perform job
	  stosleep(job_time);
	  cout << "Job completed at location: " << job_x << " " << job_y << " in time " << job_time << endl; 
	  job_time = 0; 
	  job_x = 0; 
	  job_y = 0; 
	  step_x = 0; 
	  step_y = 0; 
	  MPI_Send(zero, 1, MPI_INT, server_rank, JOB_DONE, MPI_COMM_WORLD); 
	  MPI_Recv(zero, 1, MPI_INT, server_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
	  if (stat.MPI_TAG == WORK) start();		// if more work to do, then repeat process
	}

	void xstep(int i = 1) {
		pos_x += i*(step_x/abs(step_x));          // adds or subtracts 1 depending on if step_x is +ve or -ve
                step_x = job_x - pos_x;               // recalculate number of steps left
                pos[0] = pos_x;
                pos[1] = pos_y;
                MPI_Send(pos, 2, MPI_INT, server_rank, MOVE, MPI_COMM_WORLD);         // update server so that map can be redrawn
                MPI_Recv(zero, 1, MPI_INT, server_rank, ACK, MPI_COMM_WORLD, &stat);
	}

	void ystep(int i = 1) {
		pos_y += i*(step_y/abs(step_y));
                step_y = job_y - pos_y;
                pos[0] = pos_x;
                pos[1] = pos_y;
                MPI_Send(pos, 2, MPI_INT, server_rank, MOVE, MPI_COMM_WORLD);
                MPI_Recv(zero, 1, MPI_INT, server_rank, ACK, MPI_COMM_WORLD, &stat);
	}

	void move() {
	// make the robot take a step. No obstacles and robots can go through each other (for now)
  	  if (job_time == 0) return;
	  while (step_x != 0 || step_y != 0) {		// if not at destination
		if (step_x != 0) {
		  xstep();
		  continue;
		}
		if (step_y != 0) {			// first take steps in x direction, then in y direction
		  ystep();
		  continue;
		}
	  }
	}
	
	void waitForClear(int x,int y){
	  int coord[2];
	  coord[0]=x;
	  coord[1]=y;
	  
	  MPI_Send(coord,2,MPI_INT,server_rank,LOOK,MPI_COMM_WORLD);
	  MPI_Recv(&isClear,1, MPI_INT, server_rank,ACK, MPI_COMM_WORLD,&stat);
	  while(isClear==0){
	    //cout<<x<<", "<<y<<" not clear yet"<<endl;
	    stosleep(1000);
	    MPI_Send(&coord,1,MPI_INT,server_rank,LOOK,MPI_COMM_WORLD);
	    MPI_Recv(&isClear,1, MPI_INT, server_rank,ACK, MPI_COMM_WORLD,&stat);
	  }
	}
	
	void move2() {
	    for (int i = 0; i < path_len; i++) {
	      isClear=false;
	      if (path[i] == 0) {
		waitForClear(pos_x-1,pos_y);
	        pos_x -= 1;
	      }
	      else if (path[i] == 1) {
		waitForClear(pos_x+1,pos_y);
	        pos_x += 1;
	      }
	      else if (path[i] == 2) {
		waitForClear(pos_x,pos_y-1);
	        pos_y -= 1;
	      }
	      else if (path[i] == 3) {
		waitForClear(pos_x,pos_y+1);
		pos_y += 1;
	      }
	      pos[0] = pos_x; pos[1] = pos_y;
	      MPI_Send(pos, 2, MPI_INT, server_rank, MOVE, MPI_COMM_WORLD);
	      MPI_Recv(zero, 1, MPI_INT, server_rank, ACK, MPI_COMM_WORLD, &stat);
	    }
	}
	
	void start() {
	// request a job, go there, do the job
	  getJob();
	  move2();
	  doJob();
	}

	void look() {
	    MPI_Send(zero, 1, MPI_INT, server_rank, LOOK, MPI_COMM_WORLD);
	    MPI_Recv(adjacent_pos, 4, MPI_INT, server_rank, ACK, MPI_COMM_WORLD, &stat);
	    // cout << "Robot at position: " << pos_x << " " << pos_y << ". Adjacencies: " << adjacent_pos[0] << adjacent_pos[1] << adjacent_pos[2] << adjacent_pos[3] << endl;
	}
};

#endif
