enum op_t {read, write, add, sub, noop};

class insn {
    public:
        op_t op;
        uint8_t data;
        uint32_t phy_addr;

        insn(op_t op, uint8_t data, uint32_t phy_addr) : op(op), data(data), phy_addr(phy_addr) {}
};
