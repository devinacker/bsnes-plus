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
  void  irq_process();
  
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

  DOS();
  ~DOS();

private:
  DOSSerial serial;
  
  bool fdc_dio;
};

extern DOS dos;
