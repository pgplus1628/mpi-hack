#include <mpi.h>
#include <glog/logging.h>
#include <vector>
#include <algorithm>

struct Vtype { 
  double x;
  double y;
  Vtype (double _x, double _y) : x(_x), y(_y) {}
  Vtype () {}
  std::string to_string() { return " " + std::to_string(x) + "\t" + std::to_string(y); }
};
typedef struct Vtype Vtype;


void red_func(void *in, void *inout, int *len, MPI_Datatype *dptr)
{
  Vtype * in_arr = static_cast<Vtype*>(in);
  Vtype * out_arr = static_cast<Vtype*>(inout);
  for(int i = 0 ;i < *len;i ++) {
    out_arr[i].x += in_arr[i].x;
    out_arr[i].y += in_arr[i].y;
  }
}


int main(int argc, char ** argv) {
  int rank;
  int num_rank;
  int ret;
  
  /* init */
  ret = MPI_Init(&argc, &argv);
  CHECK_EQ(ret, MPI_SUCCESS);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_rank);
  LOG(INFO) << " rank = " << rank  << " / " << num_rank;
  CHECK(rank >= 0);
  
  /* declare data type */
  MPI_Datatype Mtype;
  MPI_Type_contiguous(sizeof(Vtype), MPI_BYTE, &Mtype);
  MPI_Type_commit(&Mtype);

  /* decleare custom op */
  MPI_Op red_op;
  MPI_Op_create( red_func, true, &red_op ); 
  

  /* buffer */
  std::vector<Vtype> send_data(num_rank); // send data
  std::vector<Vtype> red_data(num_rank); // reduce result

  std::for_each(send_data.begin(), send_data.end(), 
               [&rank](Vtype &v) { v.x = (double)rank; v.y = (double)rank; });


  std::string my_data = "";
  std::for_each(send_data.begin(), send_data.end(), 
                [&my_data](Vtype &v) { my_data += v.to_string() + " | "; });

  LOG(INFO) << rank << " : >>> " << my_data;

  /* reduce */
  int root = 0;
  MPI_Reduce(send_data.data(), red_data.data(), num_rank, Mtype, red_op ,root, MPI_COMM_WORLD);

  if (rank == root) {
    std::string result = "";
    std::for_each(red_data.begin(), red_data.end(), 
                  [&result](Vtype &v) { result += v.to_string() + " | ";});
    LOG(INFO) << rank << " : ==== " << result;
  }


  /*finalize */
  ret = MPI_Finalize();
  CHECK_EQ(ret, MPI_SUCCESS);


  return 0;
}



