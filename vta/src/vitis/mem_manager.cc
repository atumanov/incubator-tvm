#include "mem_manager.h"

namespace cma_manager {
    cma_entry(uint64_t size, uint64_t offset, void* addr) :
        size{size}, offset{offset}, ptr{addr} {;}

    cma_pool::cma_pool(POOL_START_SIZE) {
        pool = static_cast<uint64_t>(malloc(POOL_START_SIZE));
    }

    void * cma_pool::alloc(uint64_t size) {
        uint64_t offset = 0;
        uint64_t addr = pool;
        for (std::vector<cma_entry>::iterator it = cma_table.begin(); it != cma_table.end(); ++it) {
            addr += it -> offset;
            offset += it -> offset;
        }
        cma_table.push_back({size, offset, reinterpret_cast<void*>(addr)});
    }

    vta_phy_addr_t cma_pool::get_physical_addr(void * ptr) {
        std::vector<cma_entry>::iterator it = cma_table.begin();
        while (it->next != NULL) {
            if (it->ptr == ptr) {
                return it->offset;
            }
        }
        return 0;
    }
};
