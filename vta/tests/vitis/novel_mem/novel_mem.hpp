enum op_t {read, write, add, sub, noop}

class insn {
    op_t op;
    uint8_t data;
    uint32_t phy_addr;
}
