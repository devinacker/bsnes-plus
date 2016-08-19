inline unsigned cache_access_speed();
inline unsigned memory_access_speed();

void add_clocks(unsigned clocks);

void rombuffer_sync();
void rombuffer_update();
uint8 rombuffer_read();

void rambuffer_sync();
uint8 rambuffer_read(uint16 addr);
void rambuffer_write(uint16 addr, uint8 data);

void timing_reset();
