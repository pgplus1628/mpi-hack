#pragma once

#include "mpi.h"
#include <glog/logging.h>
#include <gflags/gflags.h>



class DistControl { 

  public : 
  int rank;
  int num_rank;
  DistControl() {}

  void init(int argc, char **argv) 
  {
    int ret = MPI_Init(&argc, &argv);
    CHECK(ret == MPI_SUCCESS);
    ret = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    CHECK(ret == MPI_SUCCESS);
    ret = MPI_Comm_size(MPI_COMM_WORLD, &num_rank);
    CHECK(ret == MPI_SUCCESS);
    LOG(INFO) << "rank = " << rank << " /  " << num_rank;
    CHECK(rank >= 0);
  }
  
  void barrier()
  {
    int ret = MPI_Barrier(MPI_COMM_WORLD);
    CHECK_EQ(ret, MPI_SUCCESS);
  }


  ~DistControl() 
  {
    int ret = MPI_Finalize();
    CHECK_EQ(ret, MPI_SUCCESS) << "DistContro::finalize MPI_Finalize failed.";
  }

};
