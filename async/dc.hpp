#pragma once

#include "mpi.h"
#include <glog/logging.h>
#include <gflags/gflags.h>



class DistControl { 

  public : 
  int rank;
  int num_rank;
  DistControl() {}

  bool init(int &argc, char **&argv) 
  {
    int ret = MPI_Init(&argc, &argv);
    CHECK(ret == MPI_SUCCESS);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(world_, &num_rank);
    LOG(INFO) << "rank = " << rank << " /  " << num_rank;
    CHECK(rank >= 0);
  }
  
  void barrier()
  {
    int ret = MPI_Barrier(MPI_COMM_WORLD);
    CHECK_EQ(ret, MPI_SUCCESS);
  }

};
