#!/bin/bash

mpiCC -std=c++11 -o main main.cpp -lsfml-graphics -lsfml-window -lsfml-system
mpiexec -hostfile /usr/mpihosts -n `expr $1 + 1` ./main

