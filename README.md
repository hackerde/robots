# Robot Rescuers

This is a simulation of robots navigating a map to perform certain tasks. The idea is that they are trying to rescue or provide some kind of first aid to people stuck in a disaster. There are obstacles on the way which the robots avoid to reach their targets.

This project uses OpenMPI installed on a multi-node cluster. It also uses SFML to generate graphics.

Install SFML by: `sudo apt install libsfml-dev`

* Run the script to compile and run automatically: `./run.sh <number_of_robots>`

OR

* Compile `main.cpp`: `mpiCC -std=c++11 -o main main.cpp -lsfml-graphics -lsfml-window -lsfml-system`

* Run using `mpiexec`: `mpiexec -hostfile <host_file_name> -n <number_of_processes> ./main`

If run with `n` processes, the program creates `1` server and `n-1` robots with `n-1+JOBS` jobs on a `MAP_HEIGHT`x`MAP_WIDTH` grid.
`JOBS`, `MAP_HEIGHT` and `MAP_WIDTH` are defined in `server.h`.
On the map, `BLACK` indicates a free spot, `YELLOW` indicates a job, `GREEN` indicates a robot and `RED` indicates an obstacle.
The map is surrounded by an obstacle fence.

