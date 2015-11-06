#!/bin/bash

EXEC=/home/zork/loclib/openmpi-1.10.1/bin/mpiexec
HOST=machines

export LD_LIBRARY_PATH=/home/zork/loclib/openmpi-1.10.1/lib/


# ----------------------------#

APP=./main

if [ $# -eq 0 ] ; then 
  echo " usage ./run.sh <np> <bytes_mb> <tbuf_mb> <sleep_ms>"
  exit
fi

${EXEC} -hostfile $HOST -np $1 ${APP} --bytes_mb=$2 --tbuf_mb=$3 --sleep_ms=$4 --logtostderr=1 

