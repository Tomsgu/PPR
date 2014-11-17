#!/bin/sh

# mpic++ -o bin -std=c++11  main.cpp graph.cpp header.cpp mpi_util.cpp
#turn on trace
mpiCC -o ./utils/bin -std=c++11 -DLOG_TRACE main.cpp graph.cpp header.cpp mpi_util.cpp

