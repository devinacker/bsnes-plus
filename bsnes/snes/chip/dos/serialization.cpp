#ifdef DOS_CPP

void DOS::serialize(serializer& s) {
  serial.serialize(s);
  
  s.integer(fdc_dio);
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

#endif
