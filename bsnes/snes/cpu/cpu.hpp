class CPU : public Processor, public CPUcore, public PPUcounter, public MMIO {
public:
  enum : bool { Threaded = true };
  array<Processor*> coprocessors;
  alwaysinline void step(unsigned clocks);
  void synchronize_smp();
  void synchronize_ppu();
  void synchronize_coprocessor();

  uint8 pio();
  bool joylatch();
  alwaysinline bool interrupt_pending() { return status.interrupt_pending; }
  alwaysinline bool refresh() { return status.dram_refresh == 1; }

  void enter();
  void power();
  void reset();

  void serialize(serializer&);
  CPU();
  ~CPU();

private:
  #include "dma/dma.hpp"
  #include "memory/memory.hpp"
  #include "mmio/mmio.hpp"
  #include "timing/timing.hpp"

  uint8 cpu_version;

  struct Status {
    bool interrupt_pending;
    uint16 interrupt_vector;

    unsigned clock_count;
    unsigned lineclocks;

    //timing
    bool irq_lock;

    unsigned dram_refresh_position;
    unsigned dram_refresh;  //0 = not refreshed; 1 = refresh active; 2 = refresh inactive

    unsigned hdma_init_position;
    bool hdma_init_triggered;

    unsigned hdma_position;
    bool hdma_triggered;

    bool nmi_valid;
    bool nmi_line;
    bool nmi_transition;
    bool nmi_pending;
    bool nmi_hold;

    bool irq_valid;
    bool irq_line;
    bool irq_transition;
    bool irq_pending;
    bool irq_hold;

    bool reset_pending;

    //DMA
    bool dma_active;
    bool dma_pending;
    bool hdma_pending;
    bool hdma_mode;  //0 = init, 1 = run

    //MMIO
    //$2181-$2183
    uint17 wram_addr;

    //$4016-$4017
    bool joypad_strobe_latch;
    uint32 joypad1_bits;
    uint32 joypad2_bits;

    //$4200
    bool nmi_enabled;
    bool hirq_enabled, virq_enabled;
    bool auto_joypad_poll;

    //$4201
    uint8 pio;

    //$4202-$4203
    uint8 wrmpya;
    uint8 wrmpyb;

    //$4204-$4206
    uint16 wrdiva;
    uint8 wrdivb;

    //$4207-$420a
    uint9 hirq_pos;
    uint9 virq_pos;

    //$420d
    unsigned rom_speed;

    //$4212
    bool auto_joypad_active;
    bool auto_joypad_latch;
    unsigned auto_joypad_counter;

    //$4214-$4217
    uint16 rddiv;
    uint16 rdmpy;

    //$4218-$421f
    uint16 joy1;
    uint16 joy2;
    uint16 joy3;
    uint16 joy4;
  } status;

  struct ALU {
    unsigned mpyctr;
    unsigned divctr;
    unsigned shift;
  } alu;

  static void Enter();
  void op_irq();
  debugvirtual void op_step();

  friend class CPUDebugger;
};

#if defined(DEBUGGER)
  #include "debugger/debugger.hpp"
  extern CPUDebugger cpu;
#else
  extern CPU cpu;
#endif
