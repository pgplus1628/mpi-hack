#pragma once

#include "mpi.hpp"
#include "basic_types.hpp"
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <utility>
#include <functional>

class Box { 

  public : 

  struct ReqInfo {
    size_t reqid;
    Range vec_range;
  };

  // ----------------/
  // Members 
  // ----------------/
  MPI_Request *req_array;
  size_t reqs_size;
  int num_chan;
  std::vector<size_t> chanoff_vec; // channel offset
  std::vector<size_t> chansz_vec;  // number of chunks in each channel
  std::vector<ReqInfo> reqinfo_vec;  // req info vector holds every ReqInfo
  std::vector<size_t> reqiter_vec; // req id iterator for each channel
  size_t tot_req_send;

  void reset_all_iter()
  {
    std::fill_n(reqiter_vec, num_chan, 0);
    tot_req_send = 0;
  }

  Box(int num_chan_in, std::vector<size_t> chansz_in)
    : num_chan(num_chan_in), chansz_vec(chansz_in), tot_req_send(0)
  {
    reqs_size = std::accumulate(chansz_vec.begin(), chansz_vec.end(), 0);
    req_array = new MPI_Request [reqs_size];
    chanoff_vec.resize(num_chan);
    size_t tot = 0;
    for(size_t i = 0;i < num_chan;i ++) {
      chanoff_vec[i] = tot;
      tot += chansz_vec[i];
    }
    
    reqinfo_vec.resize(reqs_size);
    for(size_t i = 0;i < reqs_size; i ++) {
      reqinfo_vec[i].reqid = i;
    }

    reqiter_vec.resize(num_chan);
    reset_all_iter();
  }

  void reset()
  {
    reset_all_iter();
  }

  size_t next_iter(int chan_id)
  {
    size_t base = chanoff_vec[chan_id];
    size_t ptr = reqiter_vec[chan_id];
    DLOG(INFO) << "DEBUG next_iter chan_id = " << chan_id
              << " ptr = " << ptr
              << " chansz = " << chansz_vec[chan_id];
    if (ptr < chansz_vec[chan_id]) {
      reqiter_vec[chan_id] ++;
      return base + ptr;
    } else {
      return UINT32_MAX;
    }
  }

  void reset_iter(int chan_id)
  {
    reqiter_vec[chan_id] = 0;
  }
  
 
  // ----------------------/
  // Sender Side 
  // ----------------------/ 

  void async_send(int chan_id, void *addr, size_t bytes, Range range)
  {
    /* get reqinfo */
    size_t rid = this->next_iter(chan_id);
    CHECK_NE(rid, UINT32_MAX);
    ReqInfo& r = reqinfo_vec[rid];
    r.vec_range = range;

    /* post send */
    int rc = MPI_Isend(addr, bytes, MPI_BYTE, chan_id, TAG_DATA,
                       MPI_COMM_WORLD, &(this->req_array[r.reqid]));
    CHECK_EQ(rc, MPI_SUCCESS);
    tot_req_send ++;
  }


  void wait_all()
  {
    MPI_Request *reqs = nullptr;
    /* collect request */
    if (tot_req_send == reqs_size){
      reqs = req_array;
    } else {
      reqs = new MPI_Request [tot_req_send];
      size_t reqs_ptr = 0;
      for(int chan_id = 0; chan_id < num_chan; chan_id ++) {
        size_t it_last = reqiter_vec[chan_id];
        size_t it_base = chanoff_vec[chan_id];
        for(size_t it = 0; it < it_last + 1; it ++) {
          size_t ii = it + it_base;
          reqs[reqs_ptr++] = this->req_array[reqinfo_vec[ii].reqid];
        }
      }
    }

    CHECK(reqs != nullptr); // TODO DEBUG
    MPI_Status *status_array = new MPI_Status [tot_req_send];
    int rc = MPI_Waitall(tot_req_send, reqs, status_array);
    CHECK_EQ(rc, MPI_SUCCESS);
    delete status_array;

    /* update request */
    if (tot_req_send != reqs_size){
      size_t reqs_ptr = 0;
      for(int chan_id = 0; chan_id < num_chan; chan_id ++) {
        size_t it_last = reqiter_vec[chan_id];
        size_t it_base = chanoff_vec[chan_id];
        for(size_t it = 0; it < it_last + 1; it ++) {
          size_t ii = it + it_base;
          this->req_array[reqinfo_vec[ii].reqid] = reqs[reqs_ptr++];
        }
      }
    }
  }


  
  // ----------------------/
  // Receiver Side
  // ----------------------/ 

  void async_recv(int chan_id, void *addr, size_t bytes, Range range)
  {
    /* get reqinfo */
    size_t rid = this->next_iter(chan_id);
    CHECK_NE(rid, UINT32_MAX);
    ReqInfo& r = reqinfo_vec[rid];
    r.vec_range = range;

    /* DEBUG */
    DLOG(INFO) << " DEBUG BOX::async_recv with bytes " << bytes
              << " chan_id " << chan_id;

    /* post recv */
    int rc = MPI_Irecv(addr, bytes, MPI_BYTE, chan_id, TAG_DATA,
                       MPI_COMM_WORLD, &(this->req_array[r.reqid]) );
    CHECK_EQ(rc, MPI_SUCCESS);
    tot_req_send ++;
  }

 
  bool wait_any(size_t *chan_id, Range* range)
  {
    if (tot_req_send == 0) return false; /* if there is not req flying */
    MPI_Status status;
    int comp_indx;

    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    int rc = MPI_Waitany(reqs_size, req_array, &comp_indx, &status);

    if (unlikely(rc != MPI_SUCCESS)) {
      LOG(ERROR) << " BOX::wait_any error : "
                 << mpi_err_info(rc);
    }

    CHECK_EQ(rc, MPI_SUCCESS);
    *chan_id = status.MPI_SOURCE;
    *range = reqinfo_vec[comp_indx].vec_range;
    DLOG(INFO) << " BOX::DEBUG wait_any from " << *chan_id
              << " with range " << range->to_string()
              << " comp_indx " << comp_indx;
    /* update stat */
    tot_req_send --;
    return true;
  } 


};


