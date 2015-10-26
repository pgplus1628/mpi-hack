#!/bin/bash

EXEC=/home/zork/loclib/mpich-3.1.4/bin/mpiexec
HOST=machines

export LD_LIBRARY_PATH=/home/zork/loclib/mpich-3.1.4/lib/

APP=./main
${EXEC} -hostfile $HOST ${APP}

