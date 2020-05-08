#include "common.h"
#include "mem_manager.h"

using namespace cma_manager;

TEST_CASE("alloc/free", "[alloc/free]") {
    SECTION("basic", "[basic]") {
        // Test that memory can be allocatd and contigous
        TEST([]()->bool {
            static constexpr uint32_t long_size = sizeof(unsigned long);
            static constexpr uint32_t int_size = sizeof(unsigned int);
            static constexpr uint32_t short_size = sizeof(unsigned short);

            std::unique_ptr<cma_pool> cma;

            printf("starting alloc\n");
            void * buf1 = cma->alloc(long_size);
            printf("finished alloc\n");
            void * buf2 = cma->alloc(int_size);
            void * buf3 = cma->alloc(short_size);

            // test contiguous
            assert(reinterpret_cast<uint64_t>(buf2) == reinterpret_cast<uint64_t>(buf1) + long_size);
            assert(reinterpret_cast<uint64_t>(buf3) == reinterpret_cast<uint64_t>(buf2) + int_size);
            return true;
        });
    }
}
