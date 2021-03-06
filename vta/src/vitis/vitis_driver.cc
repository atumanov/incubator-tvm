/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file vitis_driver.cc
 * \brief VTA driver for Vitis.
 */
#include <vta/driver.h>
#include <vta/hw_spec.h>
#include <tvm/runtime/registry.h>
#include <type_traits>
#include <map>
#include <unordered_map>
#include <cstring>
#include <sstream>
#include <vta/xcl2.h>
#include "mem_manager.h"


/*
void VitisProgramBin(cl::Context& context, cl::Device& device, const char *binaryFile) {
  cl_int err;
  auto file_buf = xcl::read_binary_file(binaryFile);
  cl::Program::Binaries bins{{file_buf.data(), file_buf.size()}};
  OCL_CHECK(err, cl::Program program(context, {device}, bins, NULL, &err));
  if(err != CL_SUCCESS) {
    printf("Failed to program devcie with xclbin %s\n", binaryFile);
  }
}*/


class VTADevice {
  private:
    cl::Context context;
    cl::Device device;
    cl::Kernel vta;
    cl::CommandQueue cmd_queue;
    cl::Program program;
    cl_int err; // OCL error checking flag
    std::unique_ptr<cma_manager::cma_pool> cma;

  public:
    VTADevice() {
      cma = make_unique();
      char name[2048];

      std::vector<cl::Device> devices = xcl::get_xil_devices();
      device = devices[0]; // currently fixed with the first device
      clGetDeviceInfo(device(), CL_DEVICE_NAME, sizeof(name), name, NULL);

      printf("\t [VITIS_DEVICE_INFO] # of devices : %d \n", devices.size());
      printf("\t [VITIS_DEVICE_INFO] device name : %s \n", name);

      std::string binaryFile =
          "/nethome/kvu35/SRTML/tvm/vta/hardware/vitis/build_dir.sw_emu.xilinx_u280_xdma_201920_1/vta.xclbin";
      std::vector<unsigned char> fileBuf = xcl::read_binary_file(binaryFile);
      printf("\t [VITIS BITSTREAM INFO] file buf : %d\n", fileBuf.size());

      cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};

      OCL_CHECK(err, context = cl::Context({device}, NULL, NULL, NULL, &err));
      OCL_CHECK(err, cmd_queue = cl::CommandQueue(context, {device},
                CL_QUEUE_PROFILING_ENABLE, &err));
      OCL_CHECK(err, program = cl::Program(context, {device}, bins, NULL, &err));
      if(err != CL_SUCCESS) {
        printf("Fail to program device\n");
      } else {
        OCL_CHECK(err, vta = cl::Kernel(program, "vta", &err));
      }
    }

    int Run(vta_phy_addr_t insn_phy_addr,
            uint32_t insn_count,
            uint32_t wait_cycles) {
      cl::Event event;
      // Data must be alread loaded to host memory
      // over here, just send them & let the kernel function work
      // until it's done
      VTAGenericInsn* insn = reinterpret_cast<VTAGenericInsn*>(insn_phy_addr);
      OCL_CHECK(err, cl::Buffer instruction_buff (context,
              CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
              insn_count * sizeof(VTAGenericInsn),
              reinterpret_cast<VTAGenericInsn*>(insn_phy_addr),
              &err));
      OCL_CHECK(err, vta = cl::Kernel(program, "vta", &err));
      OCL_CHECK(err, err = vta.setArg(0, instruction_buff));
      OCL_CHECK(err, err = cmd_queue.enqueueTask(vta, NULL, &event))
      OCL_CHECK(err, err = cmd_queue.enqueueMigrateMemObjects({instruction_buff}, CL_MIGRATE_MEM_OBJECT_HOST));
      cmd_queue.finish();
      return 0;
    }
};


/* VTA Driver Functions
 *   vta/include/vta/driver.h
 */
void* VTAMemAlloc(size_t size, int cached) {
  return cma->alloc(size);
}

void VTAMemFree(void* buf) {
  cma->free(buf);
}

vta_phy_addr_t VTAMemGetPhyAddr(void* buf) {
  cma->get_physical_addr(buf);
}

void VTAMemCopyFromHost(void* dst, const void* src, size_t size) {
  //memcpy(dst, src, size);
}

void VTAMemCopyToHost(void* dst, const void* src, size_t size) {
  //memcpy(dst, src, size);
}

void VTAFlushCache(void* vir_addr, vta_phy_addr_t phy_addr, int size) {
}

void VTAInvalidateCache(void* vir_addr, vta_phy_addr_t phy_addr, int size) {
}

VTADeviceHandle VTADeviceAlloc() {
  printf("VTADeviceAlloc()\n");
  return new VTADevice();
}

void VTADeviceFree(VTADeviceHandle handle) {
  delete static_cast<VTADevice*>(handle);
}

int VTADeviceRun(VTADeviceHandle handle,
                 vta_phy_addr_t insn_phy_addr,
                 uint32_t insn_count,
                 uint32_t wait_cycles) {
  return static_cast<VTADevice*>(handle)->Run(
      insn_phy_addr, insn_count, wait_cycles);
}
