#pragma once

#include "mpi.h"

class DistControl { 

  public : 
  int rank;
  int num_rank;
  DistControl(int _rank, int _num_rank) 
    : rank(_rank), num_rank(_num_rank)
  { 
  }

  
  void barrier()
  {
    int ret = MPI_Barrier(MPI_COMM_WORLD);
    CHECK_EQ(ret, MPI_SUCCESS);
  }

};
