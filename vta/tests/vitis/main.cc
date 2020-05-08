#include "mem_manager.h"

using namespace std;
using namespace cma_manager;

void bootstrap(vector<function<bool()>>&);

static constexpr uint32_t long_size = sizeof(unsigned long);
static constexpr uint32_t int_size = sizeof(unsigned int);
static constexpr uint32_t short_size = sizeof(unsigned short);

int main() {
    vector<function<bool()>> test_suite;
    cout << "start" << endl;
    bootstrap(test_suite);

    int test_num = 0;
    for (vector<function<bool()>>::iterator it = test_suite.begin();
            it != test_suite.end();
            it++) {
        function<bool()> test = *it;
        bool res = test();
        if (!res) {
            std::cout << "test #" << test_num++ << " failed" << std::endl;
        }
    }
}

void bootstrap(vector<function<bool()>> &test_suite) {
    test_suite.push_back([]()->bool {
        unique_ptr<cma_pool> cma;

        cout << "start" << endl;
        void * buf1 = cma->alloc(long_size);
        void * buf2 = cma->alloc(int_size);
        void * buf3 = cma->alloc(short_size);

        // test contiguous
        if (reinterpret_cast<uint64_t>(buf2) != reinterpret_cast<uint64_t>(buf1) + long_size ||
                reinterpret_cast<uint64_t>(buf3) != reinterpret_cast<uint64_t>(buf2) + int_size) {
                return false;
        }
        return true;
    });
}
