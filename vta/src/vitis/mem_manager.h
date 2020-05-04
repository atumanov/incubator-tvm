#pragma once

#include <vector>

#define POOL_START_SIZE ((1 << 20) * 20) // allocate 10 MB starting out

namespace cma_manager {
class cma_entry {
    void * ptr;
    uint64_t offset;
    uint64_t size;

    cma_entry();
};

class cma_pool {
    uint64_t pool;
    std::vector<cma_entry> cma_table;

    cma_pool();
    ~cma_pool();
    void * alloc(uint64_t size);
    void free(void * ptr);
    vta_phy_addr_t get_physical_addr(void* ptr);
};
};
