#include "bus.hpp"

class Cx4 : public Coprocessor, public Memory {
public:
  unsigned frequency;
  
  static void Enter();
  void enter();
  
  //memory.cpp
  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);
  uint8 rom_read(unsigned addr);
  
  void init();
  void enable();
  void power();
  void reset();
  void serialize(serializer&);

  //data.cpp
  static const uint24 dataROM[1024];

protected:
  
  //memory.cpp
  uint8 dram_read(unsigned addr);
  void dram_write(unsigned addr, uint8 data);
  
  uint8 dsp_read(unsigned addr);
  void dsp_write(unsigned addr, uint8 data);
  
  void push();
  void pull();
  unsigned sa();
  unsigned ri();
  unsigned np();
  void instruction();

  //registers.cpp
  uint24 register_read(uint8 addr) const;
  void register_write(uint8 addr, uint24 data);

  uint8 dataRAM[3072];
  
  struct Registers {
    bool halt;

    uint24 pc;
    uint16 p;
    bool n;
    bool z;
    bool c;

    uint24 a;
    uint24 acch;
    uint24 accl;
    uint24 busdata;
    uint24 romdata;
    uint24 ramdata;
    uint24 busaddr;
    uint24 ramaddr;
    uint24 gpr[16];
  } regs;

  uint24 stack[8];
  uint16 opcode;

  struct MMIO {
    bool dma;  //true during DMA transfers

    uint24 dmaSource;       //$1f40-$1f42
    uint24 dmaLength;       //$1f43-$1f44
    uint24 dmaTarget;       //$1f45-$1f47
    uint8  r1f48;           //$1f48
    uint24 programOffset;   //$1f49-$1f4b
    uint8  r1f4c;           //$1f4c
    uint16 pageNumber;      //$1f4d-$1f4e
    uint8  programCounter;  //$1f4f
    uint8  r1f50;           //$1f50
    uint8  r1f51;           //$1f51
    uint8  r1f52;           //$1f52
    uint8  vector[32];      //$1f60-$1f7f
  } mmio;

};

extern Cx4 cx4;
extern Cx4Bus cx4bus;
