#include "mem_manager.h"

namespace cma_manager {
    cma_entry::cma_entry(uint64_t size, uint64_t offset, void* addr) :
        size{size}, offset{offset}, ptr{addr} {;}

    cma_pool::cma_pool() {
        pool = new char[POOL_START_SIZE];
        bytes_used = 0;
        capacity = POOL_START_SIZE;
    }

    cma_pool::~cma_pool() {
        delete[] pool;
    }

    void * cma_pool::alloc(uint64_t size) {
        bytes_used += size;
        if (bytes_used > capacity) {
            std::cout << "OOPS" << std::endl;
            return 0;
        }
        if (cma_table.size() == 0) {
            cma_table.push_back({size, 0, static_cast<void*>(pool)});
            return static_cast<void*>(pool);
        }
        cma_entry last_elem = cma_table.back();
        uintptr_t addr = reinterpret_cast<uintptr_t>(last_elem.ptr) + last_elem.size;
        void * addr_ptr = reinterpret_cast<void*>(addr);
        cma_table.push_back({size, last_elem.offset + last_elem.size, addr_ptr});
        return addr_ptr;
    }

    vta_phy_addr_t cma_pool::get_physical_addr(void * ptr) {
        std::vector<cma_entry>::iterator it = cma_table.begin();
        while (it != cma_table.end()) {
            if (it->ptr == ptr) {
                return it->offset;
            }
            it++;
        }
        return 0;
    }
};
