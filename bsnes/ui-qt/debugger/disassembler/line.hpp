#ifndef __DISASSEMBLER_LINE__H__
#define __DISASSEMBLER_LINE__H__

struct DisassemblerLine {
  enum Type { Empty, Opcode };

  void setEmpty() {
    type = Empty;
  }

  void setOpcode(uint32_t address, const string &text) {
    type = Opcode;
    this->address = address;
    this->text = text;
  }

  Type type;
  string text;
  uint32_t address;
};

#endif
