extern "C" {
    void vta(const void * phy_mem_w,
            const void * phy_mem_r,
            const uint insn_phy_addr,
            int num_insn) {
        int reg = 0;
        insn * curr_insn = phy_mem_r + insn_phy_addr;
        for (int i = 0; i < num_insn; i++) {
            switch (curr_insn[i].op) {
                case op_t::add :
                    reg += curr_insn[i].data;
                    break;
                case op_t::sub :
                    reg -= curr_insn[i].data;
                    break;
                case op_t::write :
                    *(reinterpret_cast<int*>(
                            static_cast<char*>(phy_mem_w) + curr_insn[i].phy_addr)) = reg;
            }
        }
    }
}
