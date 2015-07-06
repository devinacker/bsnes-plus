inline unsigned cache_access_speed();
inline unsigned memory_access_speed();

bool r15_modified;

void add_clocks(unsigned clocks);

void rombuffer_sync();
void rombuffer_update();
debugvirtual uint8 rombuffer_read();

void rambuffer_sync();
debugvirtual uint8 rambuffer_read(uint16 addr);
debugvirtual void rambuffer_write(uint16 addr, uint8 data);

void r14_modify(uint16);
void r15_modify(uint16);

void timing_reset();
