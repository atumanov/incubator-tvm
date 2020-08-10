#include "xcl2.hpp"
#include <stdlib.h>
#include <vector>
#include <algorithm>

#define MEM_SIZE (10 << 10)

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
    return EXIT_FAILURE;
  }

  std::string binaryFile = argv[1];
  size_t vector_size_bytes = sizeof(int) * DATA_SIZE;
  cl_int err;
  cl::Context context;
  cl::Kernel krnl_vector_add;
  cl::CommandQueue q;

  // OPENCL HOST CODE AREA START
  // get_xil_devices() is a utility API which will find the xilinx
  // platforms and will return list of devices connected to Xilinx platform
  auto devices = xcl::get_xil_devices();
  // read_binary_file() is a utility API which will load the binaryFile
  // and will return the pointer to file buffer.
  auto fileBuf = xcl::read_binary_file(binaryFile);
  cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
  int valid_device = 0;
  for (unsigned int i = 0; i < devices.size(); i++) {
    auto device = devices[i];
    // Creating Context and Command Queue for selected Device
    OCL_CHECK(err, context = cl::Context(device, NULL, NULL, NULL, &err));
    OCL_CHECK(err, q = cl::CommandQueue(context, device,
                                        CL_QUEUE_PROFILING_ENABLE, &err));
    std::cout << "Trying to program device[" << i
              << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
    cl::Program program(context, {device}, bins, NULL, &err);
    if (err != CL_SUCCESS) {
      std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
    } else {
      std::cout << "Device[" << i << "]: program successful!\n";
      OCL_CHECK(err, krnl_vector_add = cl::Kernel(program, "vadd", &err));
      valid_device++;
      break; // we break because we found a valid device
    }
  }

  if (valid_device == 0) {
    std::cout << "Failed to program any device found, exit!\n";
    exit(EXIT_FAILURE);
  }

  void * mem_ptr_r, mem_ptr_w;
  uint num_insn;
  uint insn_phy_addr;

  OCL_CHECK(err, cl::Buffer cl_phy_mem_r(
            context, CL_MEM_READ_ONLY,
            MEM_SIZE, NULL, &err));
  OCL_CHECK(err, cl::Buffer cl_phy_mem_w(
            context, CL_MEM_WRITE_ONLY,
            MEM_SIZE, NULL, &err));

  OCL_CHECK(err, err = krnl_vector_add.setArg(0, cl_phy_mem_w));
  OCL_CHECK(err, err = krnl_vector_add.setArg(1, cl_phy_mem_r));
  OCL_CHECK(err, err = krnl_vector_add.setArg(2, insn_phy_addr));
  OCL_CHECK(err, err = krnl_vector_add.setArg(3, num_insn));

  OCL_CHECK(err, auto mem_ptr_r = q.clEnqueueMapBuffer(&cl_phy_mem_r,
              CL_TRUE,
              CL_MAP_READ,
              0,
              MEM_SIZE,
              NULL,
              NULL,
              &err));
  OCL_CHECK(err, auto mem_ptr_w = q.clEnqueueMapBuffer(&cl_phy_mem_w,
              CL_TRUE,
              CL_MAP_WRITE,
              0,
              MEM_SIZE,
              NULL,
              NULL,
              &err));

  insn_phy_addr = rand() % (5 << 10) + (2 << 10); // store instructions between 2 and 5 kB
  insn * insn_ptr = reinterpret_cast<insn*>(mem_ptr_w + insn_phy_addr);
  num_insn = 4;
  std::vector<insn, aligned_allocator<insn>> insn_stream{
      {op_t::add, 12, 0},
      {op_t::add, 7, 0},
      {op_t::sub, 3, 0}
      {op_t::write, 0, 0}
  };
  memcpy(insn_stream.data, insn_ptr, insn_stream.size * sizeof(insn));

  OCL_CHECK(err, err = q.enqueueMigrateMemObject({mem_ptr_w, mem_ptr_r}, 0));

  // Launch the Kernel
  // For HLS kernels global and local size is always (1,1,1). So, it is
  // recommended
  // to always use enqueueTask() for invoking HLS kernel
  OCL_CHECK(err, err = q.enqueueTask(krnl_vector_add));

  // Copy Result from Device Global Memory to Host Local Memory
  OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output},
                                                  CL_MIGRATE_MEM_OBJECT_HOST));
  q.finish();
  // OPENCL HOST CODE AREA END

  std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl;
  return (match ? EXIT_SUCCESS : EXIT_FAILURE);
}
