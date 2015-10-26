#!/bin/bash

#EXEC=/home/zork/loclib/mpich-3.1.4/bin/mpiexec
EXEC=mpiexec
HOST=machines

#export LD_LIBRARY_PATH=/home/zork/loclib/mpich-3.1.4/lib/

APP=./main
#${EXEC} --mca btl openib,self,sm --hostfile $HOST ${APP} --num_int=${SZ}
${EXEC} --hostfile $HOST ${APP}
