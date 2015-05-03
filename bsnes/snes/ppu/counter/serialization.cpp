#ifdef PPU_CPP

void PPUcounter::serialize(serializer &s) {
  s.integer(status.hcounter);
  s.integer(status.vcounter);
  s.integer(status.fieldlines);
  s.integer(status.field);

  s.integer(history.index);
  s.array(history.vcounter);
  s.array(history.hcounter);
}

#endif
