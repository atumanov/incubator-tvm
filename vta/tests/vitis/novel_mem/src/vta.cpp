#include <cstdint>
#include "novel_mem.hpp"

extern "C" {
    void vta(char * phy_mem_w,
            char * phy_mem_r,
            int insn_phy_addr,
            int num_insn,
            int * out_reg) {
        int reg = 0;
        insn * curr_insn = reinterpret_cast<insn*>(phy_mem_r + insn_phy_addr);
        for (int i = 0; i < num_insn; i++) {
            switch (curr_insn[i].op) {
                case op_t::add :
                    reg += curr_insn[i].data;
                    break;
                case op_t::sub :
                    reg -= curr_insn[i].data;
                    break;
                case op_t::write :
                    *(reinterpret_cast<int*>(phy_mem_w + curr_insn[i].phy_addr)) = reg;
                    break;
                default:
                    break;
            }
        }
        //*(reinterpret_cast<int*>(phy_mem_w + 5)) = reg;
        *out_reg = reg;
    }
}
