#include <nall/file.hpp>

class BSXBase : public Coprocessor, public MMIO {
public:
  static void Enter();
  void enter();
  void init();
  void enable();
  void power();
  void reset();
  void unload();

  void serialize(serializer&);

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

private:
  struct BSXStream {
    // stream MMIO
    uint14 channel;  // $2188-2189, $218e-218f
    uint8 prefix;    // $218b, $2191
    uint8 data;      // $218c, $2192
    uint8 status;    // $218d, $2193
    
    // broadcast data packet (from bsxdat)
    file packets;
    int offset;
    uint14 loaded_channel;
    uint8 loaded_count;
    
    // internal state
    bool pf_latch, dt_latch;
    uint8 count;
    bool first;
    uint16 queue; // number of remaining unbuffered packets
    uint16 pf_queue; // number of buffered prefix bytes
    uint16 dt_queue; // number of buffered packets
    
    // Time Channel data
    struct tm time;
  };
  
  struct {
    //Stream 1 & 2
    BSXStream stream[2];
    
    //Other
    uint4 r2194;
    uint8 r2195, r2196, r2197;

    //Serial
    uint8 r2198, r2199;
	
	//Unknown
	uint8 r219a;
  } regs;
  
  bool stream_fileload(BSXStream &stream);
  uint8 get_time(BSXStream &stream);

  bool local_time;
  time_t custom_time, start_time;
};

class BSXCart : public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  void serialize(serializer&);

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

  BSXCart();
  ~BSXCart();

private:
  struct {
    uint8 r[16];
    uint8 rtemp[16];
    bool irq, irq_en;
    bool dirty;
  } regs;

  void update_memory_map();
};

class BSXFlash : public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  void serialize(serializer&);

  unsigned size() const;
  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

private:
  struct {
    unsigned short command;
    bool csr, esr;
    bool vendor_info;
    bool writebyte;
    uint8 flash_size;
  } regs;
};

extern BSXBase  bsxbase;
extern BSXCart  bsxcart;
extern BSXFlash bsxflash;
