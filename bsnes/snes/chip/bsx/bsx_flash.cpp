#ifdef BSX_CPP

BSXFlash bsxflash;

void BSXFlash::init() {}
void BSXFlash::enable() {}

void BSXFlash::power() {
  reset();
}

void BSXFlash::reset() {
  regs.command = 0;

  regs.csr = false;
  regs.esr = false;
  regs.vendor_info = false;
  regs.writebyte = false;
  regs.flash_size = min(0xC, (unsigned)log2(size() >> 10));

  memory::bsxpack.write_protect(true);
}

unsigned BSXFlash::size() const {
  return memory::bsxpack.size();
}

uint8 BSXFlash::read(unsigned addr) {
  if(regs.esr) {
    switch (addr & 0xFFFF)
    {
      case 0x0002:	//PSR, Page Status Register
        return 0xC0;

      case 0x0004:	//GSR, Global Status Register
        return 0x82;
    }
  }

  if(regs.csr)
  {
    //Read Compatible Status Register
    if(!Memory::debugger_access())
      regs.csr = false;
    return 0x80;
  }

  if(regs.vendor_info && ((addr & 0x7FFF) >= 0x7F00) && ((addr & 0x7FFF) <= 0x7F13)) {
    //read flash cartridge vendor information
    switch(addr & 0xFF) {
      case 0x00: return 0x4d;
      case 0x01: return 0x00;
      case 0x02: return 0x50;
      case 0x03: return 0x00;
      case 0x04: return 0x00;
      case 0x05: return 0x00;
      case 0x06: return 0x10 | regs.flash_size; //Memory Pack Type 1
      case 0x07: return 0x00;
      default:   return 0x00;
    }
  }

  return memory::bsxpack.read(addr);
}

void BSXFlash::write(unsigned addr, uint8 data) {
  //there exist both read-only and read-write BS-X flash cartridges ...
  //unfortunately, the vendor info is not stored inside memory dumps
  //of BS-X flashcarts, so it is impossible to determine whether a
  //given flashcart is writeable.
  //however, it has been observed that LoROM-mapped BS-X carts always
  //use read-write flashcarts, and HiROM-mapped BS-X carts always use
  //read-only flashcarts.
  //below is an unfortunately necessary workaround to this problem.
  //if(cartridge.mapper() == Cartridge::BSCHiROM) return;

  //If Write Byte Command is issued
  if(regs.writebyte)
  {
    regs.writebyte = false;
    memory::bsxpack.write(addr, memory::bsxpack.read(addr) & data);	//AND byte
    return memory::bsxpack.write_protect(true);
  }

  //Commands
  regs.command <<= 8;
  regs.command  |= data;

  //Single Byte Commands
  switch (data)
  {
    case 0x00:
    case 0xFF:
      //Reset Array Data
      regs.csr = false;
      regs.esr = false;
      regs.vendor_info = false;
      break;

    case 0x10:
    case 0x40:
      //Write Byte (next write will be )
      regs.writebyte = true;
      break;

    case 0x70:
      //Enable CSR
      regs.csr = true;
      break;

    case 0x71:
      //Enable ESR
      regs.esr = true;
      break;

    case 0x75:
      //Enable Page Buffer
      regs.vendor_info = true;
      break;
  }

  //Double Byte Commands
  switch (regs.command)
  {
    case 0x20D0:
      //Page Erase
      memory::bsxpack.write_protect(false);
      for (int i = 0; i < 0x10000; i++)
        memory::bsxpack.write((addr & 0xFF0000) + i, 0xFF);
      break;

    case 0xA7D0:
      //Chip Erase
      memory::bsxpack.write_protect(false);
      for (int i = 0; i < memory::bsxpack.size(); i++)
        memory::bsxpack.write(i, 0xFF);
      break;
  }

  memory::bsxpack.write_protect(!regs.writebyte);
}

#endif

