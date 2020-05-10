#include "mem_manager.h"
#include <memory>
#include <functional>

#define TEST(x) if (!(x)) {return false;}

using namespace std;
using namespace cma_manager;

void bootstrap(vector<function<bool()>>&);

int main() {
    vector<function<bool()>> test_suite;
    bootstrap(test_suite);

    int test_num = 0;
    for (vector<function<bool()>>::iterator it = test_suite.begin();
            it != test_suite.end();
            it++) {
        cout << "Running test ...." << test_num << endl;
        function<bool()> test = *it;
        bool res = test();
        if (!res) {
            cout << "test #" << test_num++ << " failed" << endl;
        } else {
            cout << "test #" << test_num++ << " passed" << endl;
        }
    }
}

void bootstrap(vector<function<bool()>> &test_suite) {
    constexpr uint32_t long_size = sizeof(unsigned long);
    constexpr uint32_t int_size = sizeof(unsigned int);
    constexpr uint32_t short_size = sizeof(unsigned short);

    /**
     * Test that memory is allocatable and contigous.
     */
    test_suite.push_back([]()->bool {
        unique_ptr<cma_pool> cma(new cma_pool());

        // test allocs
        char * buf1 = static_cast<char*>(cma->alloc(long_size));
        char * buf2 = static_cast<char*>(cma->alloc(int_size));
        char * buf3 = static_cast<char*>(cma->alloc(short_size));

        // test contiguous
        TEST(buf2 == &buf1[long_size] && buf3 == &buf2[int_size])

        // test correct total size
        TEST(cma->bytes_used == long_size + int_size + short_size)
        return true;
    });

    /**
     * Create a pseudoVTA test. This test follows how VTA allocates and free memories and tests
     * that these memories are accessable using offsets defined in get_physical_addr.
     */
    test_suite.push_back([]()->bool {
        unsigned int buf2_val = 0xde;
        unique_ptr<cma_pool> cma(new cma_pool());

        char * buf1 = static_cast<char*>(cma->alloc(long_size));
        char * buf2 = static_cast<char*>(cma->alloc(int_size));
        char * buf3 = static_cast<char*>(cma->alloc(short_size));

        // test correctness of the bufer offset
        TEST(buf1 == &cma->pool[cma->get_physical_addr(buf1)])
        TEST(buf2 == &cma->pool[cma->get_physical_addr(buf2)])
        TEST(buf3 == &cma->pool[cma->get_physical_addr(buf3)])

        // make sure we can use the offset to read/write data
        *(reinterpret_cast<unsigned int*>(&cma->pool[cma->get_physical_addr(buf2)])) = buf2_val;
        TEST(*(reinterpret_cast<unsigned int*>(buf2)) == buf2_val)
        return true;
    });
}
