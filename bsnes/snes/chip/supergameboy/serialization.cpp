#ifdef SUPERGAMEBOY_CPP

void SuperGameBoy::serialize(serializer &s) {
  Processor::serialize(s);
  s.integer(speed);
  if (s.mode() == serializer::Load) update_speed();
  if(sgb_serialize) sgb_serialize(s);
}

#endif
