#!/bin/bash

#EXEC=/home/zork/loclib/openmpi-1.10.1/bin/mpiexec
#export LD_LIBRARY_PATH=/home/zork/loclib/openmpi-1.10.1/lib/

EXEC=/home/zork/loclib/mpich-3.1.4/bin/mpiexec
export LD_LIBRARY_PATH=/home/zork/loclib/mpich-3.1.4/lib/

HOST=machines



# ----------------------------#

APP=./main

if [ $# -eq 0 ] ; then 
  echo " usage ./run.sh <np> <bytes_mb> <tbuf_mb> <sleep_ms>"
  exit
fi

#${EXEC} -hostfile $HOST -np $1 ${APP} --bytes_mb=$2 --tbuf_mb=$3 --sleep_ms=$4 --logtostderr=1 

#MPICH_ASYNC_PROGRESS=1 ${EXEC} -hostfile $HOST -np $1 ${APP} --bytes_mb=$2 --tbuf_mb=$3 --sleep_ms=$4 --logtostderr=1 --ring=0
MPICH_ASYNC_PROGRESS=1 ${EXEC} -hostfile $HOST -np $1 ${APP} --bytes_mb=$2 --tbuf_mb=$3 --sleep_ms=$4 --logtostderr=1 
