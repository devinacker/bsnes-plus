#ifdef PPU_CPP

void PPUcounter::serialize(serializer &s) {
  s.integer(status.hcounter);
  s.integer(status.vcounter);
  s.integer(status.lineclocks);
  s.integer(status.prev_lineclocks);
  s.array(status.fieldlines);
  s.integer(status.field);
}

#endif
