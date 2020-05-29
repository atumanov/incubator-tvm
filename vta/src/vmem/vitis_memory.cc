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
 * \file vitis_memory.cc
 * \brief Thread-safe host memory manager
 */

#include "vitis_memory.h"
#include <vta/driver.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <list>
#include <utility>
#include <iterator>
#include <unordered_map>
#include <map>
#include <mutex>

namespace vta {
namespace hmem {

void* HostMemoryManager::GetAddr(uint64_t phy_addr) {
  std::lock_guard<std::mutex> lock(mutex_);
  void *dram_base = &host_dram[0];
  //printf("[GetAddr] drambase(%p), retaddr(%p) \n", dram_base, (void*)(dram_base + phy_addr));

  return reinterpret_cast<void*>(dram_base + phy_addr);
}

vta_phy_addr_t HostMemoryManager::GetPhyAddr(void* buf) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = active_map_.find((Page*)buf);
  if(it == active_map_.end()) {
    printf("[GeyPhyAddr] page not found\n");
    return -1;
  }
  vta_phy_addr_t offset = 0;
  vta_phy_addr_t dram_base = (uint64_t)&(host_dram[0]);
  offset = reinterpret_cast<uint64_t>(buf) - dram_base;
  return offset;
}

void* HostMemoryManager::Alloc(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);

  size_t npage = (size + PAGE_SIZE - 1) / PAGE_SIZE;

  auto it = free_map_.lower_bound(npage);
  if (it != free_map_.end()) {
    Page* page_head = it->second;
    size_t num_consecutive_pages = it->first;
    active_map_.insert(std::make_pair(page_head, num_consecutive_pages));
    free_map_.erase(it);
    active_pages += npage;
    return (void *)page_head;
  }

  if(alloc_index >= (MAX_NUM_PAGES -1)) {
    printf("[Alloc] memory is full\n");
    return NULL;
  }

  active_map_[&host_dram[alloc_index]] = npage;
  void *data = &host_dram[alloc_index];
  //std::cout << "Alloc info: at " << &host_dram[alloc_index] << ", npages " << npage << std::endl;
  //printf("\t[Alloc_Before] alloc_index: %ld, active_pages: %ld, npages: %ld\n", alloc_index, active_pages, npage);
  alloc_index += npage;
  active_pages += npage;
  //printf("\t[Alloc_After] alloc_index: %ld, active_pages: %ld, npages: %ld\n", alloc_index, active_pages, npage);

  return data;
}

void HostMemoryManager::Free(void* data) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (active_map_.size() == 0) return;

  auto it = active_map_.find(reinterpret_cast<Page*>(data));
  if(it == active_map_.end()) {
    std::cout << "[Free] page not found" << std::endl;
    return;
  }
  //printf("\t [Free] active pages (before): %ld \n", active_pages);
  Page* page_head = it->first;
  size_t num_consecutive_pages = it->second;
  free_map_.insert(std::make_pair(num_consecutive_pages, page_head));
  active_map_.erase(reinterpret_cast<Page*>(data));
  active_pages -= num_consecutive_pages;
  //printf("\t [Free] active pages (after): %ld \n", active_pages);
}

// TODO
void HostMemoryManager::MemCopyFromHost(void* dst, const void * src, size_t size) {
}

// TODO
void HostMemoryManager::MemCopyToHost(void* dst, const void * src, size_t size) {
}

HostMemoryManager* HostMemoryManager::Global() {
  static HostMemoryManager inst;
  return &inst;
}

uint64_t HostMemoryManager::GetNumOfActivePages() {
  return active_pages;
}

}  // namespace hmem
}  // namespace vta
