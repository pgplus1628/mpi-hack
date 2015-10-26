#include <iostream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/time.h>
#include <time.h>
#include "mpi.h"

#define TIMER(val) do { \
  struct timeval tm; \
  gettimeofday(&tm, NULL); \
  val = tm.tv_sec * 1000 + tm.tv_usec/1000; \
} while(0)


void init(double *arr, size_t len) {
  for(size_t i = 0;i < len;i ++) {
    arr[i] = double(i);
  }
}

void validate(double *arr, size_t len) {
  
  LOG(INFO) << " validate len = " << len;
  for(size_t i = 0;i < len;i ++) {
    CHECK_EQ((size_t)(arr[i]), i);
  }
}



int main(int argc, char ** argv) {

  size_t G = 1024 * 1024 * 1024;

  MPI_Init(&argc, &argv);
  size_t len = (5 * G) / sizeof(double);

  double *send_data = new double[len];
  double *recv_data = new double[len];

  init(send_data, len);

  validate(send_data, len);
  

  MPI_Request send_req;
  MPI_Request recv_req;
  MPI_Status send_status, recv_status;

  int64_t t1,t2,t3,t4,t5;

  int rc;

  TIMER(t1);
  rc = MPI_Irecv(recv_data,  len, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &recv_req);
  CHECK_EQ(rc, MPI_SUCCESS);

  TIMER(t2);
  rc = MPI_Isend(send_data,  len, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &send_req);
  CHECK_EQ(rc, MPI_SUCCESS);

  TIMER(t3);
  rc = MPI_Wait(&send_req, &send_status);
  CHECK_EQ(rc, MPI_SUCCESS);

  TIMER(t4);
  rc = MPI_Wait(&recv_req, &recv_status);
  CHECK_EQ(rc, MPI_SUCCESS);

  TIMER(t5);
 
  LOG(INFO) << " send time : " << t4 - t2 ; 

  LOG(INFO) << " send_wait time : " << t4 - t3 ; 
  LOG(INFO) << " recv_wait time : " << t5 - t4 ; 
  LOG(INFO) << " tot time : " << t5 - t1 ; 

  size_t tot_size = len * sizeof(double);
  double d_size = (double)(tot_size) / G;
  LOG(INFO) << "d_size " << d_size << " G" ;

  //LOG(INFO) << " send_bandwidth : " << d_size / (1.0 * ( t4 - t3)) << " GB/s";
  //LOG(INFO) << " recv_bandwidth : " << d_size / (1.0 * ( t5 - t4)) << " GB/s";
  LOG(INFO) << " tot_bandwidth : "  << d_size / (1.0 * ( t5 - t1)) * 1000 << " GB/s";

  validate(recv_data, len);
 


  LOG(INFO) << " ------------- Memcpy ---------------";
  TIMER(t1);
  memcpy(recv_data, send_data, sizeof(double) * len);
  TIMER(t2);

  LOG(INFO) << " mempcy time : " << t2 - t1;
  LOG(INFO) << " memcpy bandwidth : " << d_size / (1.0 *(t2-t1)) * 1000 << " GB/s";


  MPI_Finalize();


  return 0;
}



