#include <nall/file.hpp>

class BSXBase : public MMIO {
public:
  void init();
  void enable();
  void power();
  void reset();
  void unload();

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

private:
  struct BSXStream {
    // stream MMIO
    uint14 channel;  // $2188-2189, $218e-218f
    uint8 prefix;    // $218b, $2191
    uint8 data;      // $218c, $2192
    uint8 prefix_or; // $218d, $2193
    
    // broadcast data packet (from bsxdat)
    file packets;
    
    // internal state
    bool pf_latch, dt_latch;
    uint8 count;
    bool first;
    uint16 queue;
  };
  
  struct {
    //Stream 1 & 2
    BSXStream stream[2];
    
    //Other
    uint8 r2194, r2195, r2196, r2197;

    //Serial
    uint8 r2198, r2199;

    //Time
    uint8 time_counter;
    uint8 time_hour, time_minute, time_second;
    uint8 time_weekday, time_day, time_month;
    uint16 time_year;
  } regs;
  
  void stream_fileload(BSXStream &stream);
  uint8 get_time();

  bool local_time;
  time_t custom_time, start_time;
};

class BSXCart : public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

  BSXCart();
  ~BSXCart();

private:
  struct {
    uint8 r[16];
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

  unsigned size() const;
  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

private:
  struct {
    unsigned short command;
    bool csr, esr;
    bool vendor_info;
    bool writebyte;
  } regs;
};

extern BSXBase  bsxbase;
extern BSXCart  bsxcart;
extern BSXFlash bsxflash;
