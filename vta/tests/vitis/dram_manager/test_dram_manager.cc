#include <catch2/catch.hpp>
#include <memory>
#include <vitis_memory.h>
//#include "/nethome/jheo33/github/tvm/vta/src/vmem/vitis_memory.h"
using namespace std;
using namespace vta;

SCENARIO("host memory manager behavior senarios", "[dram_manager]") {
  GIVEN("A host dram manger instance") {
    using MemoryManager = vta::hmem::HostMemoryManager;
    MemoryManager* manager = MemoryManager::Global();
    static constexpr uint32_t long_size = sizeof(unsigned long);
    static constexpr uint32_t int_size = sizeof(unsigned int);
    static constexpr uint32_t short_size = sizeof(unsigned short);

    void *buf1, *buf2, *buf3, *buf4;

    WHEN("The host dram manager allocates new memeory in page granularity") {
      buf1 = manager->Alloc(long_size);    // page 1
      buf2 = manager->Alloc(int_size);     // page 2
      buf3 = manager->Alloc(PAGE_SIZE+1);  // page 3,4
      buf4 = manager->Alloc(short_size);   // page 5

      THEN("check memory contiguity, free, and realloc") {
        REQUIRE(reinterpret_cast<uint64_t>(buf2) == \
            reinterpret_cast<uint64_t>(buf1) + PAGE_SIZE);
        REQUIRE(reinterpret_cast<uint64_t>(buf3) == \
            reinterpret_cast<uint64_t>(buf2) + PAGE_SIZE);
        REQUIRE(reinterpret_cast<uint64_t>(buf4) == \
            reinterpret_cast<uint64_t>(buf3) + (PAGE_SIZE * 2));
        REQUIRE(manager->GetNumOfActivePages() == 5);

        manager->Free(buf3); // free page 3,4
        REQUIRE(manager->GetNumOfActivePages() == 3);

        buf3 = manager->Alloc(PAGE_SIZE+100);
        REQUIRE(reinterpret_cast<uint64_t>(buf3) == \
            reinterpret_cast<uint64_t>(buf2) + PAGE_SIZE);
        REQUIRE(manager->GetNumOfActivePages() == 5);

        manager->Free(buf1);
        manager->Free(buf2);
        manager->Free(buf3);
        manager->Free(buf4);
      }
    }

    WHEN("The host dram manager allocates new memeory in page granularity") {
      buf1 = manager->Alloc(long_size);    // page 1
      buf2 = manager->Alloc(int_size);     // page 2
      buf3 = manager->Alloc(PAGE_SIZE+1);  // page 3,4
      buf4 = manager->Alloc(short_size);   // page 5

      THEN("check physical address <--> virtual address conversion"){
        uint64_t buf1_phy = manager->GetPhyAddr(buf1);
        uint64_t buf2_phy = manager->GetPhyAddr(buf2);
        uint64_t buf3_phy = manager->GetPhyAddr(buf3);
        uint64_t buf4_phy = manager->GetPhyAddr(buf4);
        REQUIRE(buf1_phy == 0);
        REQUIRE(buf2_phy == PAGE_SIZE);
        REQUIRE(buf3_phy == (PAGE_SIZE * 2));
        REQUIRE(buf4_phy == (PAGE_SIZE * 4));

        REQUIRE(buf1 == manager->GetAddr(buf1_phy));
        REQUIRE(buf2 == manager->GetAddr(buf2_phy));
        REQUIRE(buf3 == manager->GetAddr(buf3_phy));
        REQUIRE(buf4 == manager->GetAddr(buf4_phy));


        unsigned int buf2_val = 0xde;
        *(reinterpret_cast<unsigned int*>(manager->GetAddr(buf2_phy))) = buf2_val;
        REQUIRE(*(reinterpret_cast<unsigned int*>(buf2)) == buf2_val);

        manager->Free(buf1);
        manager->Free(buf2);
        manager->Free(buf3);
        manager->Free(buf4);
      }
    }
  }
}
