#include "MemoryPool.h"

MemoryPool::MemoryPool(size_t capacity) {
    OCL_CHECK(err, cl::Buffer dev_read_ptr(context,
                CL_MEM_READ_ONLY,
                capacity * PAGE_SIZE,
                nullptr,
                &err));
    OCL_CHECK(err, cl::Buffer dev_write_ptr(context,
                CL_MEM_WRITE_ONLY,
                capacity * PAGE_SIZE,
                nullptr,
                &err));
    OCL_CHECK(err, phy_w = q.enqueueMapBuffer(dev_read_ptr,
                    CL_TRUE,
                    CL_MAP_WRITE,
                    0,
                    MEM_SIZE,
                    NULL,
                    NULL,
                    &err));
    OCL_CHECK(err, phy_r = q.enqueueMapBuffer(dev_write_ptr,
                    CL_TRUE,
                    CL_MAP_READ,
                    0,
                    MEM_SIZE,
                    NULL,
                    NULL,
                    &err));
    free_list.push_back({0, capacity, nullptr}); // insert the initial node
}

void * MemoryPool::alloc(size_t num_pages) {
    std::vector<page_cluster>::iterator ptr;
    page_cluster chk_out_node = nullptr;
    for (ptr = free_list.begin(); ptr < free_list.end(); ptr++) {
        if (ptr.size == num_pages) {
            chk_out_node = *ptr;
            free_list.remove(*ptr);
        } else if (ptr.size > num_pages) {
            chk_out_node = page_cluster(*ptr);
            ptr->size -= num_pages;
            ptr->pg_offset += num_pages;
            chk_out_node.size = num_pages;
        }
    }
    return static_cast<void*>(static_cast<char*>(phy_w) + chk_out_node.pg_offset * PGSIZE);
}
