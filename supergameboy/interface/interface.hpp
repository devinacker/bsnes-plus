class SuperGameBoy : public gambatte::InputGetter, public gambatte::DebugHandler {
public:
  gambatte::GB *gambatte_;

//SuperGameBoy::MMIO
  static void scanline(unsigned line);
  unsigned vram_row;
  uint8_t vram[320*4];

  struct MMIO {
    uint8_t write_buf;
    uint8_t read_buf;
    uint8_t r6003;
    uint8_t r6004;
    uint8_t r6005;
    uint8_t r6006;
    uint8_t r6007;
    uint8_t r7000[16];
    unsigned read_pos;
    uint8_t mlt_req;
  } mmio;

//SuperGameBoy::Packet
  static const char command_name[32][64];

  struct Packet {
    uint8_t data[16];
    uint8_t& operator[](unsigned addr) { return data[addr & 15]; }
  };
  Packet packet[64];
  unsigned packetsize;

  unsigned joyp_id;
  bool joyp_lock;
  bool pulselock;
  bool strobelock;
  bool packetlock;
  Packet joyp_packet;
  uint8_t packetoffset;
  uint8_t bitdata, bitoffset;

  void joyp_write(bool p15, bool p14);

//SuperGameBoy::Core
  uint8_t *romdata, *ramdata, *rtcdata;
  unsigned romsize,  ramsize,  rtcsize;
  static const uint8_t bootroms[2][256];
  bool version;

  bool init(bool version);
  void term();
  const uint8_t* bootrom();
  unsigned run(uint32_t *samplebuffer, unsigned samples);
  void save();
  void serialize(nall::serializer &s);
  void power();
  void reset();
  uint8_t read(uint16_t addr);
  uint8_t read_gb(uint16_t addr);
  void write(uint16_t addr, uint8_t data);
  void write_gb(uint16_t addr, uint8_t data);

  void mmio_reset();
  void render();

  SuperGameBoy();
  ~SuperGameBoy();
  
  uint32_t *buffer;

//gambatte::InputGetter
  unsigned operator()();
};

extern SuperGameBoy supergameboy;
