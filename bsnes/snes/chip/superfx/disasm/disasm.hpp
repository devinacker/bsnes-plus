void disassemble_opcode(char *output, uint32 addr, bool track_regs = false);
void disassemble_alt0(char *output, uint32 addr);
void disassemble_alt1(char *output, uint32 addr);
void disassemble_alt2(char *output, uint32 addr);
void disassemble_alt3(char *output, uint32 addr);

uint16 disassemble_regs, disassemble_lastregs;