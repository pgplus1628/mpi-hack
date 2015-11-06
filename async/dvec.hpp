#pragma once 
#include <vector>
#include <utility>

#include "box.hpp"
#include "dc.hpp"

#define TBufMB (1)

typedef std::pair<size_t, size_t> Range;


template<typename T>
class DistVec { 

  public : 
  std::vector<std::vector<T>* > loc_data;
  size_t chunk_size;
  DistControl &dc;
  Box box;
  size_t chunk_bytes;
  size_t k_chunk_ele;
  std::vector<Range> * chunk_range_array; /* chunk range */
  std::vector<size_t>  size_vec;
  size_t num_chan;


  DistVec(DistControl _dc, std::vector<size_t> size_vec_in) 
    : dc( _dc), size_vec(size_vec_in)
  {
    num_chan = dc.num_rank; 
  }

  init() 
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

    /* fill size */



  }





};


