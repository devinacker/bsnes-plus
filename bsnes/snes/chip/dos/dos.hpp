class DOSSerial {
public:
  void reset();

  // CPU interface ($5f0x)
  uint8 read(uint3 addr);
  void write(uint3 addr, uint8 data);
  
  // peripheral->CPU
  void send_data(bool channel, uint8 data);
  
  void serialize(serializer&);
  
  DOSSerial();
  ~DOSSerial();

private:
  struct Channel {
    uint8 rx_buffer[3];
	uint8 rx_count;
	bool  rx_overflow;
	bool  rx_enable;
	uint3 rx_irq_enable;
	bool  rx_irq_pending;
	
	uint8 tx_buffer;
	uint8 tx_count;
	bool  tx_underflow;
	bool  tx_enable;
	bool  tx_irq_enable;
	bool  tx_irq_pending;
	
    uint8 nreg;
  } channels[2];
  
  bool irq_enable;
  
  void  reset(bool channel);
  uint8 irq_status() const;
  
  friend class DOS;
};

class DOSFloppy {
public:
  void reset();

  // CPU interface ($5f2x)
  uint8 read(bool addr);
  void write(bool addr, uint8 data);
  void terminate();
  
  void serialize(serializer&);
  
  DOSFloppy();
  ~DOSFloppy();

private:
  struct upd765_t *fdc;
  struct fdd_t *fdd;

  // uPD765 callbacks
  static int seek_track(int drive, int track, void* user_data);
  int seek_track(int drive, int track);
  static int seek_sector(int drive, struct upd765_sectorinfo_t* info, void* user_data);
  int seek_sector(int drive, struct upd765_sectorinfo_t* info);
  static int read_sector(int drive, uint8_t h, void* user_data, uint8_t* data);
  int read_sector(int drive, uint8_t h, uint8_t* data);
  static int write_sector(int drive, uint8_t h, void* user_data, uint8_t data);
  int write_sector(int drive, uint8_t h, uint8_t data);
  static int track_info(int drive, int side, void* user_data, struct upd765_sectorinfo_t* info);
  int track_info(int drive, int side, struct upd765_sectorinfo_t* info);
  static void drive_info(int drive, void* user_data, struct upd765_driveinfo_t* info);
  void drive_info(int drive, struct upd765_driveinfo_t* info);
  static void irq_set(void* user_data, bool status);
  void irq_set(bool status);

  bool irq_pending;
  bool irq_status() const;
  
  friend class DOS;
};


class DOS : public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  void serialize(serializer&);

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

  // see nall::Keyboard::Scancode
  void send_key(unsigned scancode, bool on);

  // update pending serial/floppy IRQs
  void irq_process();

  DOS();
  ~DOS();

private:
  DOSSerial serial;
  DOSFloppy floppy;
};

extern DOS dos;
