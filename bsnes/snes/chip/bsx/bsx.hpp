#include <nall/file.hpp>

class BSXBase : public MMIO {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

private:
  file SAT1;
  file SAT2;

  void stream1_fileload(uint8 count);
  void stream2_fileload(uint8 count);
  uint8 get_time(bool reset);

  bool local_time;
  time_t custom_time, start_time;

  struct {
    //Stream 1
    uint8 r2188, r2189, r218b, r218c, r218d;

    //Stream 2
    uint8 r218e, r218f, r2191, r2192, r2193;

    //Other
    uint8 r2194, r2195, r2196, r2197;

    //Serial
    uint8 r2198, r2199;

    //Time
    uint8 time_counter;
    uint8 time_hour, time_minute, time_second;
    uint8 time_weekday, time_day, time_month;
    uint8 time_yearL, time_yearH;

    //Internal
    bool pf_latch1_enable, dt_latch1_enable;
    bool pf_latch2_enable, dt_latch2_enable;

    bool stream1_loaded, stream2_loaded;
    uint8 stream1_count, stream2_count;
    bool stream1_first, stream2_first;
    uint16 stream1_queue, stream2_queue;
  } regs;
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
