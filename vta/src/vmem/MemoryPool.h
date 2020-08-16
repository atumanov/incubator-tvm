#pragma once

class page_cluster {
    public:
        uint64_t pg_offset; // # pg_offset
        size_t size; // # of pages available

        page_cluster(uint64_t pg_offset, size_t num_pages, page_cluster* next) :
            pg_offset{pg_offset}, num_pages{num_pages}, next{next};
}

class MemoryPool {
    public:
        void * phy_r, * phy_w; // # the phyical start of the memory pool
        size_t pool_capacity; // # of pages the pool contains
        size_t free_space; // # of free pages
        std::list<page_cluster> free_list; // the free list, contains all the free pages

        MemoryPool(size_t);
        void * alloc(size_t);
        void free(void *);
}
