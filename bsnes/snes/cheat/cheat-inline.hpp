bool Cheat::active() const { return cheat_enabled; }
bool Cheat::exists(uint16 addr) const { return bitmask[addr >> 3] & 1 << (addr & 7); }
