#pragma once

#include <vector>
#include <inttypes.h>
#include <iterator>
#include <iostream>
#include <functional>
#include <memory>
#include "driver.h"

#define POOL_START_SIZE ((1 << 20) * 200) // allocate 200 MB starting out

using namespace std;

namespace cma_manager {
class cma_entry {
    public:
        void * ptr;
        uint64_t offset;
        uint64_t size;

        cma_entry(uint64_t size, uint64_t offset, void* addr);
};

class cma_pool {
    public:
        char* pool;
        vector<cma_entry> cma_table;
        uint64_t bytes_used;
        uint64_t capacity;

        cma_pool();
        ~cma_pool();
        void * alloc(uint64_t size);
        void free(void * ptr);
        vta_phy_addr_t get_physical_addr(void* ptr);
};
};
