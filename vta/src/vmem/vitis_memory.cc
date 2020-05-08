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

#include <dmlc/logging.h>
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

vta_phy_addr_t HostMemoryManager::GetPhyAddr(void* buf) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = active_map_.find((Page*)buf);
  vta_phy_addr_t offset = 0;
  vta_phy_addr_t dram_base = vta_phy_addr_t(&host_dram[0]);
  offset = (vta_phy_addr_t)buf - dram_base;
  return offset;
}

void* HostMemoryManager::Alloc(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);

  size_t npage = (size + PAGE_SIZE - 1) / PAGE_SIZE;

  auto it = free_map_.lower_bound(npage);
  if (it != free_map_.end()) {
    Page* p = it->second;
    free_map_.erase(it);
    return p->data;
  }

  CHECK(alloc_index >= (MAX_NUM_PAGES -1));

  active_map_[&host_dram[alloc_index]] = npage;
  void *data = &host_dram[alloc_index];
  alloc_index += npage;

  return data;
}

void HostMemoryManager::Free(void* data) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (active_map_.size() == 0) return;

  auto it = active_map_.find(data);
  CHECK(it != active_map_.end());

  Page* page_head = it->first.get();
  size_t num_consecutive_pages = it->second.get()

  free_map_.insert(std::make_pair(num_consecutive_pages, page_head));
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

}  // namespace hmem
}  // namespace vta
