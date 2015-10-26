#include <vector>
#include <string>
#include <mpi.h>
#include <glog/logging.h>

struct Edge {
  int src;
  int dst;
  double val;
  Edge(int _src, int _dst, double _val)
    :src(_src), dst(_dst), val(_val){}
  Edge(){}
  std::string to_string(){
    return std::to_string(src) + " " + std::to_string(dst) + " " + std::to_string(val);
  }

}__attribute__((packed));

typedef struct Edge Edge;




template<typename T>
void all2all(const std::vector<T>& send_data, std::vector<size_t> send_offset, 
             std::vector<T>& recv_data, std::vector<size_t> &recv_offset, size_t num_procs)
{
  /* all to all vector size */
  CHECK_EQ(send_offset.size(), num_procs);
  recv_offset.resize(num_procs);
  std::vector<int> send_buffer_sizes(num_procs);
  std::vector<int> recv_buffer_sizes(num_procs);
  std::vector<int> send_buffer_offset(num_procs);
  std::vector<int> recv_buffer_offset(num_procs);
  int tot_send = 0;
  for(int i = 0;i < num_procs - 1;i ++) {
    send_buffer_sizes[i] = (send_offset[i+1] - send_offset[i]) * sizeof(T);
    send_buffer_offset[i] = tot_send;
    tot_send += send_buffer_sizes[i];
  }

  send_buffer_sizes[num_procs - 1] = (send_data.size() - send_offset[num_procs - 1]) * sizeof(T);
  send_buffer_offset[num_procs - 1] = tot_send;

  int rc = MPI_Alltoall(send_buffer_sizes.data(), 1, MPI_INT,
                       recv_buffer_sizes.data(), 1, MPI_INT, MPI_COMM_WORLD);
  CHECK_EQ(rc, MPI_SUCCESS) << "MPI_Alltoall failed when scatter buffer sizes.";

  /* construct offset */
  recv_buffer_offset = recv_buffer_sizes;
  int tot_recv = 0;
  for(int i = 0;i < num_procs;i ++) {
    int tmp = recv_buffer_offset[i];
    recv_buffer_offset[i] = tot_recv;
    tot_recv += tmp;
    recv_offset[i] = recv_buffer_offset[i] / sizeof(T);
  }

  /* do send */
  recv_data.resize(tot_recv / sizeof(T));
  rc = MPI_Alltoallv(send_data.data(), send_buffer_sizes.data(), send_buffer_offset.data(), MPI_BYTE,
                     recv_data.data(), recv_buffer_sizes.data(), recv_buffer_offset.data(), MPI_BYTE,
                     MPI_COMM_WORLD);
  CHECK_EQ(rc, MPI_SUCCESS); 
}

template<typename T>
std::string vec_to_string(std::vector<T> send_data, std::vector<size_t> send_offset)
{
  std::string d_str ;
  size_t pt = 0;
  for(int i = 0;i < send_data.size();i ++) {
    if (i == send_offset[pt]) {
      d_str += " \n ";
      pt ++;
    }
    d_str += send_data[i].to_string() + " " ;
  }
  return d_str;
}



int main(int argc, char ** argv)
{
  int rank;
  int num_procs;
  /* init */
  int ret = MPI_Init(&argc, &argv);
  CHECK_EQ(ret, MPI_SUCCESS);    

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  LOG(INFO) << "rank = " << rank << " / " << num_procs;
  CHECK(rank >= 0);

  /* send user defined vector */ 
  std::vector<Edge> send_data;
  std::vector<size_t> send_offset;
  std::vector<Edge> recv_data;
  std::vector<size_t> recv_offset; 

  int tot_send = 0;
  for(int i = 0;i < num_procs;i ++) {
    send_offset.push_back(tot_send);
    size_t send_size = i + 1;
    for(int j = 0;j < send_size ;j ++) {
      send_data.emplace_back(rank, i, (double)j);
    }
    tot_send += send_size;
  }
 
  all2all<Edge>(send_data, send_offset, recv_data, recv_offset, num_procs);

  std::string send_str = vec_to_string<Edge>(send_data, send_offset);
  LOG(INFO) << rank << " send : " << send_str;

  std::string recv_str = vec_to_string<Edge>(recv_data, recv_offset); 
  LOG(INFO) << rank << " recv : " << recv_str;

 
  /* finalize */
  ret = MPI_Finalize();
  CHECK_EQ(ret, MPI_SUCCESS);

  return 0;
}


