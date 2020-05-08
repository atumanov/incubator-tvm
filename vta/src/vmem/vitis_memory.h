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
 * \file vitis_memory.h
 * \brief Vitis host memory manager for OCL memory model compatibility
 */

#ifndef VTA_VITIS_MEMORY_H_
#define VTA_VITIS_MEMORY_H_

#include <vta/driver.h>
#include <cstdint>
#include <type_traits>
#include <mutex>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

namespace vta {
namespace hmem {

#define DRAM_SIZE 536870912 // 512MB initially
#define PAGE_SIZE 4096      // 4KB Pagesize
#define MAX_NUM_PAGES 131072

typedef unsigned char BYTE;

typedef struct Page {
	BYTE data[PAGE_SIZE];
} Page;

typedef struct PageTableElem {
  Page* page_head;
  size_t num_consecutive_pages;
} PageTableElem;

class HostMemoryManager {
 public:
  HostMemoryManager() {
    alloc_index = 0;
    host_dram = std::vector<Page>(MAX_NUM_PAGES);
  }
  void *GetAddr(uint64_t phy_addr);
  vta_phy_addr_t GetPhyAddr(void* buf);
  void* Alloc(size_t size);
  void Free(void* data);
  void MemCopyFromHost(void* dst, const void * src, size_t size);
  void MemCopyToHost(void* dst, const void * src, size_t size);
  static HostMemoryManager* Global();

 private:
  std::mutex mutex_;
  std::vector<Page> host_dram; // DRAM with pages
  uint64_t alloc_index;
	std::map<Page*, size_t> active_map_; // DRAM page tabel; active-page table
  std::multimap<size_t, Page*> free_map_; // mapping of free pages
};

}  // namespace hmem
}  // namespace vta

#endif  // VTA_VITIS_MEMORY_H_
