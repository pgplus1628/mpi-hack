#include <mpi.h>
#include <vector>
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
std::string vec_to_string(std::vector<T> vec)
{
  std::string d_str;
  for(auto i : vec) {
    d_str += " " + std::to_string(i);
  }
  return d_str;
}


template<typename T>
void all_gather(std::vector<T>& send_data, std::vector<T>& recv_data, std::vector<size_t> &recv_offset, size_t num_procs)
{
  int rc;

  /* all to all send_data size */
  std::vector<int> send_size(num_procs);
  std::vector<int> recv_size(num_procs);
  for(auto &i : send_size) { i = sizeof(T) * send_data.size(); }

  rc = MPI_Alltoall(send_size.data(), 1, MPI_INT,
                    recv_size.data(), 1, MPI_INT, MPI_COMM_WORLD);
  CHECK_EQ(rc, MPI_SUCCESS);

  /* offset */
  std::vector<int> recv_buffer_offset(num_procs);
  recv_offset.resize(num_procs);
  int tot_recv = 0;
  for(int i = 0;i < num_procs;i ++) {
    recv_buffer_offset[i] = tot_recv;
    tot_recv += recv_size[i];
    recv_offset[i] = recv_buffer_offset[i] / sizeof(T);
  }
  recv_data.resize( tot_recv / sizeof(T));
  rc = MPI_Allgatherv(send_data.data(), sizeof(T) * send_data.size(), MPI_BYTE,
                      recv_data.data(), recv_size.data(), recv_buffer_offset.data(), MPI_BYTE, MPI_COMM_WORLD);

  CHECK_EQ(rc, MPI_SUCCESS);
}


template<typename T>
void shuffle(std::vector<std::vector<T> *> &send_data, std::vector<T> &recv_data)
{
  /* declare type */
  MPI_Datatype Mtype;
  MPI_Type_contiguous(sizeof(T), MPI_BYTE, &Mtype);
  MPI_Type_commit(&Mtype);

  /* compact to one vec */
  size_t num_node = send_data.size();
  std::vector<T> send_buffer;
  std::vector<int> send_buffer_size(num_node, 0);
  std::vector<int> recv_buffer_size(num_node, 0);
  std::vector<int> send_buffer_offset(num_node, 0);
  std::vector<int> recv_buffer_offset(num_node, 0);

  size_t tot_send_ele = 0;
  for(auto vec : send_data) { tot_send_ele += vec->size(); }
  size_t send_ptr = 0;
  send_buffer.resize(tot_send_ele);

  size_t it = 0;;
  for(auto vec : send_data) {
    std::copy(vec->begin(), vec->end(), send_buffer.begin() + send_ptr);
    send_buffer_size[it] = vec->size() ;
    send_buffer_offset[it] = send_ptr ;
    send_ptr += vec->size();
    it ++;
  }


  /* all to all size */
  int rc = MPI_Alltoall(send_buffer_size.data(), 1, MPI_INT,
                        recv_buffer_size.data(), 1, MPI_INT, MPI_COMM_WORLD);
  CHECK_EQ(rc, MPI_SUCCESS) << "MPI_Alltoall failed when scatter buffer sizes.";  

  /* construct offset */

  recv_buffer_offset = recv_buffer_size;
  size_t tot_recv = 0;
  for(size_t i = 0;i < num_node; i ++) {
    size_t tmp = recv_buffer_offset[i];
    recv_buffer_offset[i] = tot_recv;
    tot_recv += tmp;
  }

  /* all to all data */
  recv_data.resize(tot_recv);

  rc = MPI_Alltoallv(send_buffer.data(), send_buffer_size.data(), send_buffer_offset.data(), Mtype,
                     recv_data.data(), recv_buffer_size.data(), recv_buffer_offset.data(), Mtype,
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


template<typename T>
std::string vecx_to_string(std::vector<T> &vec) 
{
  std::string d_str;
  for(auto &e : vec ) {
    d_str += e.to_string() + " " ;
  }
  return d_str;
}




void test_shuffle(int rank, int nprocs)
{
  std::vector<std::vector<Edge> *> send_data(nprocs);
  std::vector<Edge> recv_data;

  for(int node = 0; node < nprocs; node ++ ) {
    size_t num_to_send = node + 1;
    send_data[node] = new std::vector<Edge>(num_to_send);
    for(auto & e : *(send_data[node])) {
      e.src = rank;
      e.dst = node;
      e.val = (double) rank;
    }
  }

  shuffle<Edge>(send_data, recv_data);
  std::string recv_str = vecx_to_string<Edge>(recv_data);

  LOG(INFO) << rank << " recv_data ----------- \n " <<  recv_str << " \n ------------------------ \n";

}



int main(int argc, char **argv)
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

  /* init buffer */
  std::vector<Edge> send_data(rank+1);
  std::vector<Edge> recv_data;
  std::vector<size_t> recv_offset;
  std::vector<size_t> send_offset(1, 1);
  
  for(auto & edge : send_data){
    edge.src = rank;
    edge.dst = rank;
    edge.val = (double)rank;
  }

  /* all gatherv */
  all_gather(send_data, recv_data, recv_offset, num_procs);


  std::string send_str = vec_to_string<Edge>(send_data, send_offset);
  LOG(INFO) << rank << " send : " << send_str;

  std::string recv_str = vec_to_string<Edge>(recv_data, recv_offset);
  LOG(INFO) << rank << " recv : " << recv_str;


  /* test shuffle */
  LOG(INFO) << " ---------------  TEST SHUFFLE ---------------------" ;
  test_shuffle(rank, num_procs);


  /* finalize */
  ret = MPI_Finalize();
  CHECK_EQ(ret, MPI_SUCCESS);

  return 0;
}

