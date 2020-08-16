#include "xcl2.hpp"
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <cstdint>
#include "novel_mem.hpp"

#define MEM_SIZE (10 << 10)

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string binaryFile = argv[1];
    cl_int err;
    cl::Context context;
    cl::Kernel krnl_vta;
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
            OCL_CHECK(err, krnl_vta = cl::Kernel(program, "vta", &err));
            valid_device++;
            break; // we break because we found a valid device
        }
    }

    if (valid_device == 0) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

    char * host_read_ptr, *host_write_ptr;
    int insn_phy_addr;

    OCL_CHECK(err, cl::Buffer dev_read_ptr(
                context, CL_MEM_READ_ONLY,
                MEM_SIZE, nullptr, &err));
    OCL_CHECK(err, cl::Buffer dev_write_ptr(
                context, CL_MEM_WRITE_ONLY,
                MEM_SIZE, nullptr, &err));

    OCL_CHECK(err, host_write_ptr = static_cast<char*>(q.enqueueMapBuffer(dev_read_ptr,
                CL_TRUE,
                CL_MAP_WRITE,
                0,
                MEM_SIZE,
                NULL,
                NULL,
                &err)));
    OCL_CHECK(err, host_read_ptr = static_cast<char*>(q.enqueueMapBuffer(dev_write_ptr,
                CL_TRUE,
                CL_MAP_READ,
                0,
                MEM_SIZE,
                NULL,
                NULL,
                &err)));

    insn_phy_addr = rand() % (5 << 10) + (2 << 10); // store instructions between 2 and 5 kB
    std::vector<insn, aligned_allocator<insn>> insn_stream{
        {op_t::add, 12, 0},
        {op_t::add, 7, 0},
        {op_t::sub, 3, 0},
        {op_t::write, 0, 0}
    };
    memcpy(host_write_ptr + insn_phy_addr, insn_stream.data(), insn_stream.size() * sizeof(insn));

    int reg;
    OCL_CHECK(err, cl::Buffer buffer_output(
                context,
                CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                sizeof(int), &reg, &err));

    OCL_CHECK(err, err = krnl_vta.setArg(0, dev_write_ptr));
    OCL_CHECK(err, err = krnl_vta.setArg(1, dev_read_ptr));
    OCL_CHECK(err, err = krnl_vta.setArg(2, insn_phy_addr));
    OCL_CHECK(err, err = krnl_vta.setArg(3, static_cast<int>(insn_stream.size())));
    OCL_CHECK(err, err = krnl_vta.setArg(4, buffer_output));

    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({dev_read_ptr, dev_write_ptr}, 0));

    OCL_CHECK(err, err = q.enqueueTask(krnl_vta));

    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({dev_write_ptr, buffer_output},
                CL_MIGRATE_MEM_OBJECT_HOST));
    q.finish();

    std::cout << *(reinterpret_cast<int*>(host_read_ptr)) << std::endl;
    std::cout << reg << std::endl;

    return 1;
}
