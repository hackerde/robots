#ifndef _SERVER_
#define _SERVER_

#include <iostream>
using namespace std;
#include "mpi.h"
#include <random>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <stack>

#define MAXSMSG 100
#define MAP_HEIGHT 15
#define MAP_WIDTH 15
#define JOBS 5
#define OBSTACLES 25

enum MapState {
  NOT_VISITED, VISITED			// keeping track while finding a path
};

enum JobState {
  INCOMPLETE, ASSIGNED, COMPLETE	// possible states of a job
};

enum MapObjects {
  EMPTY, JOB, ROBOT, OBSTACLE		// objects in a map
};

enum MessageType { 
  JOB_REQUEST, ACK, LOOK, JOB_DONE, INIT, MOVE, WORK	// different types of message requests
};

enum Direction {
  UP, DOWN, LEFT, RIGHT			// directions to try while finding a path
};


class Server {
 public:

  int map[MAP_HEIGHT+2][MAP_WIDTH+2][2];	// map is the 10x10 grid
  int ** jobs;		// jobs[id][0,1,2,3] = x_coord, y_coord, time, status 
  int ** robots;	// robots[id][0,1,2] = x_coord, y_coord, job_id
  stack <int> path;	// to calculate the path for each robot
  int total_procs;	// number of robots+1 (for the server)
  int * arr_path;	// an array containing the steps a robot should take to reach the destination

  bool isEmpty(int x, int y) {
  // used while placing obstacles and jobs
	if (map[x][y][0] == EMPTY && map[x][y][1] == NOT_VISITED) return true;
	else return false;
  }

  bool isEmpty(int x, int y, int t) {
  // used while finding paths to jobs
	if (map[x][y][1] == NOT_VISITED) {
	  if (map[x][y][0] == EMPTY) return true;
	  if (map[x][y][0] == JOB) {
	    for (int i = 0; i < t+JOBS; i++) {
	      if (jobs[i][0] == x && jobs[i][1] == y && jobs[i][3] == COMPLETE) return true;
	    }
	  }
	  if (map[x][y][0] == ROBOT) return true;
	}
	return false;
  }

  void processStack(stack <int> &s) {
  // convert a stack to an array in the reverse order
    int size = s.size();
    arr_path = new int[size];
    for (int i = size-1; i >= 0; i--) {
	arr_path[i] = s.top();
	s.pop();
    } 
  }

  int nearestJob(int x, int y, int t) {
  // find the job nearest to the robot at position x,y; t is total number of jobs
	int x1, y1, job_id, distance = MAP_HEIGHT+MAP_WIDTH;
	for (int i = 0; i < t; i++) {
	  if (jobs[i][3] == INCOMPLETE) {
	    x1 = abs(jobs[i][0] - x);
	    y1 = abs(jobs[i][1] - y);
	    if (x1+y1 < distance) { distance = x1+y1; job_id = i; }
	  }
	}
	return job_id;
  }

  void init(int total) {
	// initialize map, jobs and robots. #jobs = #robots+JOBS. Also create obstacles at random places.

	// initialize map
	for (int i = 0; i < MAP_HEIGHT+2; i++) {
	  for (int j = 0; j < MAP_WIDTH+2; j++) {
		if (i == 0 || j == 0 || i == MAP_HEIGHT+1 || j == MAP_WIDTH+1)
		    map[i][j][0] = OBSTACLE;
		else
		    map[i][j][0] = EMPTY;
		map[i][j][1] = NOT_VISITED;
	  }
	}

	int counter = 0;
	while (counter < total) {
	    map[counter/MAP_WIDTH+1][counter%MAP_WIDTH+1][0] = ROBOT;		// keeps placing robots row-wise from top left
	    robots[counter][0] = counter/MAP_WIDTH+1;
	    robots[counter][1] = counter%MAP_WIDTH+1;
	    robots[counter][2] = -1;
	    counter++;
	}

	srand(time(0));

	int j_x, j_y; 
	// place obstacles at random places. TO FIX: obstacles may completely surround robots
	counter = 0;
	while (counter < OBSTACLES) {
	  j_x = (int)(rand())%MAP_HEIGHT+1;
	  j_y = (int)(rand())%MAP_WIDTH+1;
	  if (isEmpty(j_x, j_y)) {
	    map[j_x][j_y][0] = OBSTACLE;
	    counter++;
	  }
	}

	// randomly place jobs
	counter = 0;
	while (counter < total+JOBS) {
	  j_x = (int)(rand())%MAP_HEIGHT+1;
	  j_y = (int)(rand())%MAP_WIDTH+1;
	  if (isEmpty(j_x, j_y) && (isEmpty(j_x+1,j_y) || isEmpty(j_x-1,j_y) || isEmpty(j_x,j_y-1) || isEmpty(j_x,j_y+1))) {		// if nothing is there and job is accessible from atleast one direction, place job, otherwise, recalculate job position. TO FIX: very rare edge cases where a job can be surrounded by obstacles leaving an empty place
  	      jobs[counter][0] = j_x;
	      jobs[counter][1] = j_y;
	      jobs[counter][2] = (int)((rand())%10+1)*1000;	// 1 to 10 seconds per job
	      jobs[counter][3] = INCOMPLETE;
	      map[j_x][j_y][0] = JOB;
	      counter++;
	  }
	}

/*	// print out all jobs
	for (int i = 0; i < total+JOBS; i++) {
	    cout << "Row: " << jobs[i][0] << " Column: " << jobs[i][1] << " Time: " << jobs[i][2] << " Status: " << jobs[i][3] << endl;
	}
*/
  }

  void printMap(sf::RenderWindow *window) {
  // to make a GUI using SFML to print out the map and display jobs, obstacles, robots and their movements
    sf::RectangleShape **shape;
    shape = new sf::RectangleShape *[MAP_HEIGHT+2];
    for (int i = 0; i < MAP_HEIGHT+2; i++)
      shape[i] = new sf::RectangleShape [MAP_WIDTH+2];

  // 30x30 boxes. RED - OBSTACLE, BLACK - EMPTY, YELLOW - JOB, GREEN - ROBOT

    for (int i = 0; i < MAP_HEIGHT+2; i++) {
      for (int j = 0; j < MAP_WIDTH+2; j++) {
	shape[i][j].setSize(sf::Vector2f(30, 30));
	shape[i][j].setOutlineColor(sf::Color::White);
	shape[i][j].setOutlineThickness(1);
	shape[i][j].setPosition(30*j+20, 30*i+20);
	if (map[i][j][0] == EMPTY)
	  shape[i][j].setFillColor(sf::Color::Black);
	else if (map[i][j][0] == JOB)
	  shape[i][j].setFillColor(sf::Color::Yellow);
	else if (map[i][j][0] == ROBOT)
	  shape[i][j].setFillColor(sf::Color::Green);
	else if (map[i][j][0] == OBSTACLE)
	  shape[i][j].setFillColor(sf::Color::Red);
	window->draw(shape[i][j]);
      }
    }
    window->display();
    window->clear();
  }

  void makeDT(int a, int b, int a_g, int b_g, int *dt) {
  // determine the preferred directions of movement at every position
    int xstep = a_g - a, ystep = b_g - b;
    if (abs(ystep) > abs(xstep)) {	// preferred direction is the one where more steps need to be taken
      if (ystep > 0) { dt[0] = RIGHT; dt[2] = LEFT; }
      else { dt[0] = LEFT; dt[2] = RIGHT; }
      if (xstep > 0) { dt[1] = DOWN; dt[3] = UP; }
      else { dt[1] = UP; dt[3] = DOWN; }
    } else {
      if (ystep > 0) { dt[1] = RIGHT; dt[3] = LEFT; }
      else { dt[1] = LEFT; dt[3] = RIGHT; }
      if (xstep > 0) { dt[0] = DOWN; dt[2] = UP; }
      else { dt[0] = UP; dt[2] = DOWN; }
    }
  }

  bool DFS(int x, int y, int x_g, int y_g) {
    if (x == x_g && y == y_g) return true;	// return in reached goal
    if ((x+1 == x_g && y == y_g) || (x-1 == x_g && y == y_g) || (x == x_g && y-1 == y_g) || (x == x_g && y+1 == y_g)) ;		// do not return false if adjacent to goal
    else if (!(isEmpty(x-1,y,total_procs-1) || isEmpty(x+1,y),total_procs-1 || isEmpty(x,y-1,total_procs-1) || isEmpty(x,y+1,total_procs-1))) return false;	// return false if adjacent to any other job than goal
    int DT[4];
    makeDT(x, y, x_g, y_g, DT);		// make the direction table

    for (int i = 0; i < 4; i++) {	// iterate through every direction
	int dir = DT[i];
	switch ((enum Direction) dir) {
	  case UP:
	    if (isEmpty(x-1,y,total_procs-1) || (y == y_g && x-1 == x_g)) {	// check if possible to take step or goal is right there
		path.push(UP);		// store the direction of movement
		map[x][y][1] = VISITED;	// mark on map
		if (!DFS(x-1,y,x_g,y_g)) path.pop();	// if cannot find job then remove the last direction of movement stored
		else return true;	// else return and store the state of the stack
	    }
	    break;

	  case DOWN:
	    if (isEmpty(x+1,y,total_procs-1) || (y == y_g && x+1 == x_g)) {
		path.push(DOWN);
		map[x][y][1] = VISITED;
		if (!DFS(x+1,y,x_g,y_g)) path.pop();
		else return true;
	    }
	    break;

	  case LEFT:
	    if (isEmpty(x,y-1,total_procs-1) || (x == x_g && y-1 == y_g)) {
		path.push(LEFT);
		map[x][y][1] = VISITED;
		if (!DFS(x,y-1,x_g,y_g)) path.pop();
		else return true;
	    }
	    break;

	  case RIGHT:
	    if (isEmpty(x,y+1,total_procs-1) || (x == x_g && y+1 == y_g)) {
		path.push(RIGHT);
		map[x][y][1] = VISITED;
		if (!DFS(x,y+1,x_g,y_g)) path.pop();
		else return true;
	    }
	    break;
	}
    }
  }
  
  int run() {

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Robot Rescuers");
    int job_id, flag, count, r, x1, y1, pos[4];		// variables to use
    bool a;						// to store DFS result

    MPI_Comm_size(MPI_COMM_WORLD, &total_procs);

    robots = new int*[total_procs-1];	// 1 server and rest are robots
    for (int i = 0; i < total_procs - 1; i++)
	robots[i] = new int[3];		// robot array as described above
    jobs = new int*[total_procs-1+JOBS];
    for (int i = 0; i < total_procs-1+JOBS; i++) {
	jobs[i] = new int[4];		// jobs array as described above
    }

    int buff[MAXSMSG+1];		// buffer to receive message
    MPI_Status status;

    init(total_procs-1);
    printMap(&window);

    while (1) {
      MPI_Recv(buff, MAXSMSG, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	r = status.MPI_SOURCE-1;	// Since server is rank 0 and robots start from rank 1 but robot array index starts from 0
      
      switch ((enum MessageType) status.MPI_TAG) {
        case JOB_REQUEST:

	  // robot asks for a job request
	  for (int i = 0; i < total_procs-1+JOBS; i++) {
		if (jobs[i][3] == INCOMPLETE) {		// if there are incomplete jobs
		    job_id = nearestJob(robots[r][0], robots[r][1], total_procs-1+JOBS);	// find nearest job
		    robots[r][2] = job_id;	// assign job
		    MPI_Send(jobs[job_id], 4, MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD);	// send all 4 elements of job[i]
		    a = DFS(robots[r][0], robots[r][1], jobs[job_id][0], jobs[job_id][1]);	// find path to job
		    if (a) {		// if path was found
		      int s = path.size();
	 	      processStack(path);	// convert stack to int array
		      for (int i = 0; i < MAP_HEIGHT+2; i++) {
	    		  for (int j = 0; j < MAP_WIDTH+2; j++)
			    map[i][j][1] = NOT_VISITED;		// remark map for next job request
	 	      }
		      MPI_Send(arr_path, s, MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD);
		      delete [] arr_path;	// clear array to store next path
		    } else { int fail[1]={-1}; MPI_Send(fail,1,MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD); }	// if could not find path, send -1
		    jobs[job_id][3] = ASSIGNED;		// mark job as assigned
		    break;
		}
	  }
	  break;
	case INIT:
	  // initialize a robot on the first row
	  MPI_Send(robots[r], 3, MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD); 
	  break;
	case JOB_DONE:
	  // robot reports a completed job
	  job_id = robots[r][2];	// find which job
	  jobs[job_id][3] = COMPLETE;			// mark is as complete
	  robots[r][2] = -1;		// mark robot as free (-1)
	  int empty[1];  empty[0] = 0;
	  flag = 0;
	  for (int i = 0; i < total_procs-1+JOBS; i++) {
		if (jobs[i][3] == INCOMPLETE){		// if jobs remain to be assigned
		  MPI_Send(empty, 1, MPI_INT, status.MPI_SOURCE, WORK, MPI_COMM_WORLD);		// tell it there are more jobs
		  flag = 1;
		  break;
		}
	  }
	  if (flag == 0) {
      	  	MPI_Send(empty, 1, MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD); 	// otherwise acknowledge job completion and do nothing else
	  }
   	  break;
	case LOOK:
	  {
	    int isClear;
	    //TODO: look from the robot's spot and check if the spot in whatever direction they're looking is another robot
	    //if there's another robot, tell em to wait
	    if(map[buff[0]][buff[1]][0]==ROBOT){//TODO: temporarily switched x and y
	      isClear=0;
	    }else{
	      isClear=1;
	    }
	    
	    MPI_Send(&isClear, 1, MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD);
	  }
  	  break;
	case MOVE:
	  // robot reports its new position after making a move

	  for (int i = 0; i < total_procs-1+JOBS; i++) {		// was the robot over an incomplete job and was just passing by?
		if (jobs[i][0] == robots[r][0] && jobs[i][1] == robots[r][1] && jobs[i][3] != COMPLETE) {
	  	  map[robots[r][0]][robots[r][1]][0] = JOB;
		  break;
		}
	  }

	  if (map[robots[r][0]][robots[r][1]][0] == ROBOT) map[robots[r][0]][robots[r][1]][0] = EMPTY;
	  
	  for (int i = 0; i < total_procs-1; i++) {		// was there another robot at the position?
		if (i == r) continue;
		else {
			if (robots[i][0] == robots[r][0] && robots[i][1] == robots[r][1]) {
			    map[robots[r][0]][robots[r][1]][0] = ROBOT;
			    break;
			}
		}
	  }
	
	  robots[r][0] = buff[0];	// set the new position
	  robots[r][1] = buff[1];
	  MPI_Send(empty, 1, MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD);		// acknowledge position update
	  map[robots[r][0]][robots[r][1]][0] = ROBOT;	// update the map
	  printMap(&window);	// only (re)draw map when a robot moves
	  break;
	default: {
	  char msg[] = "Unrecognized type ";
	  cout << msg << status.MPI_TAG << endl;
	  MPI_Send(msg, strlen(msg), MPI_CHAR, status.MPI_SOURCE, ACK, MPI_COMM_WORLD);
        }
      }
	// has all jobs been completed? Abort if yes.
      count = 0;
      for (int i = 0; i < total_procs-1+JOBS; i++) {
	if (jobs[i][3] == COMPLETE) count++;
      }
      if (count == total_procs-1+JOBS) {
	cout << "All jobs completed." << endl;
//	MPI_Abort(MPI_COMM_WORLD, 0);
      }
    }
  }
};

#endif
