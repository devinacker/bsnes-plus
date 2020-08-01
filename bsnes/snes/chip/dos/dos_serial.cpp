#ifdef DOS_CPP

DOSSerial::DOSSerial() {
  reset();
}

DOSSerial::~DOSSerial() {
}

void DOSSerial::reset() {
  reset(0);
  reset(1);
  
  irq_enable = false;
}

void DOSSerial::reset(bool channel) {
  memset(&channels[channel], 0, sizeof(channels[0]));
}

uint8 DOSSerial::read(uint3 addr) {
  auto& channel = channels[!(addr & 2)];
  uint8 data = 0;
  
  if (addr & 1) {
    // reading data from peripheral
    data = channel.rx_buffer[0];
    if (channel.rx_count
        && !Memory::debugger_access()) {
      channel.rx_buffer[0] = channel.rx_buffer[1];
      channel.rx_buffer[1] = channel.rx_buffer[2];
      channel.rx_buffer[2] = 0;
      channel.rx_count--;

      if (!channel.rx_count)
        channel.rx_irq_pending = false;
      dos.irq_process();
    }
  } else {
    // reading register
    switch (channel.nreg) {
    case 0x00:
      data = ((!!channel.rx_count) << 0)
           | ((!channel.tx_count) << 2);
      break;

    case 0x01:
      data = (channel.rx_overflow << 5);
      break;

    case 0x03:
      data = irq_status();
      break;
    }

    if (!Memory::debugger_access())
      channel.nreg = 0;
  }

  return data;
}

void DOSSerial::write(uint3 addr, uint8 data) {
  if (Memory::debugger_access())
     return;

  auto& channel = channels[!(addr & 2)];
  
  if (addr & 1) {
    // writing data to peripheral (TODO)
  } else if (channel.nreg == 0) {
    // selecting register
    channel.nreg = data & 7;

    // SCC command codes
    switch ((data >> 3) & 7) {
    case 1: // point high
      channel.nreg |= 8;
      break;
    
    case 6: // error reset
      channel.rx_overflow = false;
      break;
    
    case 7: // interrupt reset
      dos.irq_process();
      break;
    }
    
  } else {
    // writing to register
    switch (channel.nreg) {
    case 1: // TX/RX interrupt control
      channel.tx_irq_enable = (data & 0x2);
      channel.rx_irq_enable = (data >> 3) & 0x3;
      break;
    
    case 3: // RX control
      channel.rx_enable = (data & 0x1);
      break;
    
    case 5: // TX control
      channel.tx_enable = (data & 0x8);
      break;
    
    case 9: // master interrupt control
      irq_enable = (data & 0x8);
      if (data & 0x80) reset(0);
      if (data & 0x40) reset(1);
      dos.irq_process();
      break;
    }

    channel.nreg = 0;
  }
}

void DOSSerial::send_data(bool num, uint8 data) {
  auto& channel = channels[num];
  
  if (channel.rx_enable) {
    if (channel.rx_count < 3) {
      channel.rx_buffer[channel.rx_count++] = data;
    } else {
      channel.rx_overflow = true;
    }
    if (channel.rx_irq_enable) {
      channel.rx_irq_pending = true;
      dos.irq_process();
    }
  }
}

uint8 DOSSerial::irq_status() const {
  return (channels[1].tx_irq_pending << 1)
       | (channels[1].rx_irq_pending << 2)
       | (channels[0].tx_irq_pending << 4)
       | (channels[0].rx_irq_pending << 5);
}

#endif
