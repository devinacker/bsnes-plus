#ifdef DOS_CPP

void DOS::serialize(serializer& s) {
  serial.serialize(s);
  floppy.serialize(s);
}

void DOSSerial::serialize(serializer& s) {
  for (auto &channel : channels) {
    s.array(channel.rx_buffer);
    s.integer(channel.rx_count);
    s.integer(channel.rx_overflow);
    s.integer(channel.rx_enable);
    s.integer(channel.rx_irq_enable);
    s.integer(channel.rx_irq_pending);

    s.integer(channel.tx_buffer);
    s.integer(channel.tx_count);
    s.integer(channel.tx_underflow);
    s.integer(channel.tx_enable);
    s.integer(channel.tx_irq_enable);
    s.integer(channel.tx_irq_pending);

    s.integer(channel.nreg);
  }

  s.integer(irq_enable);
}

void DOSFloppy::serialize(serializer& s) {
  s.integer(fdc->phase);
  s.integer(fdc->cmd);
  s.integer(fdc->fifo_pos);
  s.integer(fdc->fifo_num);
  s.array(fdc->fifo);

  s.integer(fdc->sector_info.physical_track);
  s.integer(fdc->sector_info.c);
  s.integer(fdc->sector_info.h);
  s.integer(fdc->sector_info.r);
  s.integer(fdc->sector_info.n);
  s.integer(fdc->sector_info.st1);
  s.integer(fdc->sector_info.st2);

  s.integer(fdc->drive_info.physical_track);
  s.integer(fdc->drive_info.sides);
  s.integer(fdc->drive_info.head);
  s.integer(fdc->drive_info.ready);
  s.integer(fdc->drive_info.write_protected);
  s.integer(fdc->drive_info.fault);

  s.array(fdc->st);
  s.integer(fdc->pins);
  s.integer(fdc->status);
}

#endif
