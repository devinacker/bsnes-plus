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
  uint8 ram_read(unsigned addr);
  void ram_write(unsigned addr, uint8 data);
  
  void init();
  void enable();
  void power();
  void reset();
  void serialize(serializer&);

  //data.cpp
  static const uint24 dataROM[1024];

protected:
  void add_clocks(unsigned);
  
  //memory.cpp
  uint8 op_read(unsigned addr);
  void op_write(unsigned addr, uint8 data);
  
  uint8 dsp_read(unsigned addr);
  void dsp_write(unsigned addr, uint8 data);
  
  void push();
  void pull();
  unsigned sa();
  unsigned ri();
  unsigned np();
  void instruction();
  void nextpc();
  void change_page();
  void load_page(uint8 cachePage, uint16 programPage);

  //registers.cpp
  uint24 register_read(uint8 addr);
  void register_write(uint8 addr, uint24 data);

  uint8 dataRAM[3072];
  
  struct Registers {
    bool halt;
    uint8 cachePage;
    bool irqPending;
    
    uint24 rwbusaddr;
    uint8 rwbustime;
    bool writebus;
    uint24 writebusdata;

    uint23 pc;
    uint15 p;
    bool n;
    bool z;
    bool v;
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
    
    uint24 mdr;
  } regs;

  uint23 stack[8];
  uint16 opcode;

  struct MMIO {
    bool dma;  //true during DMA transfers
    bool suspend;
    bool cacheLoading;

    uint24 dmaSource;       //$1f40-$1f42
    uint24 dmaLength;       //$1f43-$1f44
    uint24 dmaTarget;       //$1f45-$1f47
    uint8  cachePreload;    //$1f48
    uint24 programOffset;   //$1f49-$1f4b
    uint16 pageNumber;      //$1f4d-$1f4e
    uint8  programCounter;  //$1f4f
    uint8  romSpeed;        //$1f50
    uint8  ramSpeed;        //$1f50
    uint8  irqDisable;      //$1f51
    uint8  r1f52;           //$1f52
    uint8  suspendCycles;   //$1f55-$1f5c
    uint8  vector[32];      //$1f60-$1f7f
  } mmio;

  struct CachePage {
    bool lock;
    uint15 pageNumber;
    uint16 data[256];
  } cache[2];

  alwaysinline bool bus_access() {
    // cartridge bus in use
    return mmio.dma || mmio.cacheLoading || regs.rwbustime > 0;
  }
  
  alwaysinline bool busy() {
    // performing DMA, cache preload, or running code
    return mmio.dma || mmio.cacheLoading || !regs.halt;
  }

};

extern Cx4 cx4;
extern Cx4Bus cx4bus;
