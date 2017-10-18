#ifndef __DISASSEMBLER_LINE__H__
#define __DISASSEMBLER_LINE__H__

struct DisassemblerParam {
  enum Type { Address, Value, Register };

  static DisassemblerParam createAddress(uint32_t value, uint32_t address) {
    DisassemblerParam param;
    param.type = Address;
    param.value = value;
    param.address = address;
    return param;
  }

  static DisassemblerParam createValue(uint32_t value) {
    DisassemblerParam param;
    param.type = Value;
    param.value = value;
    return param;
  }

  static DisassemblerParam createRegister(const char *name) {
    DisassemblerParam param;
    param.type = Register;
    param.text = name;
    return param;
  }

  Type type;
  uint32_t value;
  uint32_t address;
  string text;
};

struct DisassemblerLine {
  enum Type { Empty, Opcode };

  enum Flag {
    FLAG_BRA = 0x01,
    FLAG_RETURN = 0x02
  };

  DisassemblerLine() : flags(0) {}

  void setEmpty() {
    type = Empty;
  }

  void setOpcode(uint32_t address, const string &text) {
    type = Opcode;
    this->address = address;
    this->text = text;

    paramFormat = "";
    params.reset();
  }

  void setBra(uint32_t target) {
    flags |= FLAG_BRA;
    targetAddress = target;
  }

  bool isEmpty() const {
    return type == Empty;
  }

  bool hasAddress() const {
    return type == Opcode;
  }

  bool isBra() const {
    return flags & FLAG_BRA;
  }

  bool isReturn() const {
    return flags & FLAG_RETURN;
  }

  Type type;
  string text;
  string paramFormat;
  uint32_t address;
  uint32_t targetAddress;
  uint8_t flags;
  linear_vector<DisassemblerParam> params;
};

#endif
