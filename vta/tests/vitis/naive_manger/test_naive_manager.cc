#include <catch2/catch.hpp>
#include <memory>
#include <mem_manager.h>
using namespace std;
using namespace cma_manager;

SCENARIO("Naive memory manager behavior senarios", "[naive_manager]") {
  GIVEN("A cma manger instance") {
    std::unique_ptr<cma_pool> cma(new cma_pool());
    static constexpr uint32_t long_size = sizeof(unsigned long);
    static constexpr uint32_t int_size = sizeof(unsigned int);
    static constexpr uint32_t short_size = sizeof(unsigned short);

    WHEN("The cma manager allocates new memeory") {
      void * buf1 = cma->alloc(long_size);
      void * buf2 = cma->alloc(int_size);
      void * buf3 = cma->alloc(short_size);

      THEN("The allocated memory is contiguous") {
        REQUIRE(reinterpret_cast<uint64_t>(buf2) == \
            reinterpret_cast<uint64_t>(buf1) + long_size);
        REQUIRE(reinterpret_cast<uint64_t>(buf3) == \
            reinterpret_cast<uint64_t>(buf2) + int_size);
        REQUIRE(cma->bytes_used == long_size + int_size + short_size);
      }

      THEN("The allocated memory is referenced via an offset from the memory pool head") {
        REQUIRE(buf1 == &cma->pool[cma->get_physical_addr(buf1)]);
        REQUIRE(buf2 == &cma->pool[cma->get_physical_addr(buf2)]);
        REQUIRE(buf3 == &cma->pool[cma->get_physical_addr(buf3)]);
      }
      THEN("It is possible to read/write data with a memory offset") {
        unsigned int buf2_val = 0xde;
        *(reinterpret_cast<unsigned int*>(
              &cma->pool[cma->get_physical_addr(buf2)])) = buf2_val;
        REQUIRE(*(reinterpret_cast<unsigned int*>(buf2)) == buf2_val);
      }
    }
  }
}
