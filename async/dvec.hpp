#pragma once
#include <vector>
#include <utility>
#include <functional>
#include <algorithm>

#include "box.hpp"
#include "dc.hpp"
#include "basic_types.hpp"

#define TBufMB (1)






template<typename T>
class DistVec {

  public :
  std::vector<std::vector<T>* > loc_data;
  size_t chunk_size;
  DistControl &dc;
  Box *box;
  size_t chunk_bytes;
  size_t k_chunk_ele;
  std::vector<std::vector<Range>*>  chunk_ranges; /* chunk range */
  std::vector<size_t>  size_vec; /* chunk size vec */
  size_t num_chan;
  Role role;

  size_t flying_recv;

  int64_t wait_any_ms;
  int64_t wait_all_ms;


  DistVec(DistControl _dc, std::vector<size_t> size_vec_in, Role _role)
    : dc( _dc), size_vec(size_vec_in), role(_role)
  {
    num_chan = dc.num_rank;
    wait_any_ms = 0;
    wait_all_ms = 0;
  }

  void init()
  {
    /* init data */
    loc_data.resize(num_chan);
    for(size_t i = 0;i < num_chan;i ++) {
      loc_data[i] = new std::vector<T>(size_vec[i]);
    }

    /* resize chunk_bytes */
    size_t ele_bytes = sizeof(T);
    chunk_bytes = TBufMB * (1024 * 1024);
    if (ele_bytes > chunk_bytes) chunk_bytes = ele_bytes;
    size_t k_chunk_ele = chunk_bytes / ele_bytes;
    chunk_bytes = k_chunk_ele * ele_bytes;

    /* fill chunk_range vec */
    chunk_ranges.resize(num_chan);
    for(size_t cid = 0;cid < num_chan; cid ++) {
      size_t size = size_vec[cid];
      size_t num_chunk = size / k_chunk_ele;
      auto &chunk_range = chunk_ranges[cid];
      chunk_range = new std::vector<Range>(num_chunk);

      size_t beg = 0;
      for(size_t chunk_id = 0; chunk_id < num_chunk-1; chunk_id ++) {
        chunk_range[chunk_id] = std::make_pair(beg, beg + k_chunk_ele);
        beg += k_chunk_ele;
      }
      chunk_range[num_chunk-1] = std::make_pair(beg, size);
    }

    /* init box */
    std::vector<size_t> num_chunks(num_chan);
    std::transform(chunk_ranges.begin(), chunk_ranges.end(), num_chunks.begin(),
      [](const std::vector<size_t> &e) { return e.size(); });
    box = new Box(num_chan, num_chunks);

    /* update status */
    flying_recv = 0;
  }

  void do_asend(int chan_id, size_t chunk_id )
  {
    CHECK_LT(chan_id, num_chan);
    CHECK_LT(chunk_id, chunk_ranges[chan_id]->size());
    auto &vec = (*loc_data[chan_id]);
    Range range = (*(chunk_ranges[chan_id]))[chunk_id];
    void *addr = vec.data() + range.first;
    size_t bytes = sizeof(T) * (range.second - range.first);
    box->async_send(chan_id, addr, bytes, range);
  }


  void post_receives()
  {
    box->reset();
    for(size_t chan_id = 0; chan_id < num_chan; chan_id ++) {
      auto & chunk_range = *(chunk_ranges[chan_id]);
      auto &vec = *(loc_data[chan_id]);

      for( auto range : chunk_range ) {
        void *addr = vec.data() + range.first;
        size_t bytes = sizeof(T) * (range.second - range.first);
        box->async_recv(chan_id, addr, bytes, range);
        flying_recv += 1;
      }

    }
  }


  void wait_all()
  {
    int64_t t1, t2;
    TIMER(t1);
    box->wait_all();
    TIMER(t2);
    wait_all_ms += (t2 - t1);
  }


  bool wait_any(int *chan_id, size_t * chunk_id)
  {
    bool ret;
    int64_t t1, t2;
    TIMER(t1);
    Range range;
    ret = box->wait_any(chan_id, &range);
    *chunk_id = range.first / k_chunk_ele;
    TIMER(t2);
    wait_any_ms += (t2 - t1);
    return ret;
  }

  std::string get_metrics(std::string name)
  {
    std::string info = name;
    info += " wait_any_ms " + std::to_string(wait_any_ms);
    info += " wait_all_ms " + std::to_string(wait_all_ms);
    return info;
  }


};
