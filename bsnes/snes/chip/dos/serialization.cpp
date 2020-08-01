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
  s.integer(upd->phase);
  s.integer(upd->cmd);
  s.integer(upd->fifo_pos);
  s.integer(upd->fifo_num);
  s.array(upd->fifo);

  s.integer(upd->sector_info.physical_track);
  s.integer(upd->sector_info.c);
  s.integer(upd->sector_info.h);
  s.integer(upd->sector_info.r);
  s.integer(upd->sector_info.n);
  s.integer(upd->sector_info.st1);
  s.integer(upd->sector_info.st2);

  s.integer(upd->drive_info.physical_track);
  s.integer(upd->drive_info.sides);
  s.integer(upd->drive_info.head);
  s.integer(upd->drive_info.ready);
  s.integer(upd->drive_info.write_protected);
  s.integer(upd->drive_info.fault);

  s.array(upd->st);
  s.integer(upd->pins);
  s.integer(upd->status);
}

#endif
