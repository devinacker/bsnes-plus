#ifdef DOS_CPP

DOS dos;

DOS::DOS() {
}

DOS::~DOS() {
}

void DOS::init() {
  reset();
}

void DOS::enable() {
  bus.map(Bus::MapMode::Direct, 0x00, 0x3f, 0x5f00, 0x5fff, *this);
  bus.map(Bus::MapMode::Direct, 0x80, 0xbf, 0x5f00, 0x5fff, *this);

  // terrible hack to patch the NMI and IRQ vectors in the standard
  // SFX-DOS ROM, since they normally have bogus addresses for some reason
  if (memory::cartrom.size() == 0x8000
      && memory::cartrom[0x7FEA] == 0x10 && memory::cartrom[0x7FEB] == 0x01
      && memory::cartrom[0x7FEE] == 0x14 && memory::cartrom[0x7FEF] == 0x01) {
	
	uint8 *cartrom = memory::cartrom.data();
    // patch NMI
    cartrom[0x7FEA] = 0x22;
    cartrom[0x7FEB] = 0xE1;
    // patch IRQ
    cartrom[0x7FEE] = 0x46;
    cartrom[0x7FEF] = 0xE1;
  }
}

void DOS::power() {
  reset();
}

void DOS::reset() {
  serial.reset();
  fdc_dio = 0;
}

uint8 DOS::read(unsigned addr) {
  if(!Memory::debugger_access())
    cpu.synchronize_coprocessor();

  switch ((addr & 0x30) >> 4) {
  case 0: // serial controller (keyboard, RS-232)
    return serial.read(addr & 3);

  case 1: // parallel controller (printer) (TODO)
    return 0;

  case 2: // floppy disk controller (TODO)
    if (!(addr & 1))
      return 0x80 | (fdc_dio << 6); // always keep RQM bit set for now so we can boot
    break;
  
  case 3: // floppy disk controller, terminal count (TODO)
    break;
  }

  return cpu.regs.mdr;
}

void DOS::write(unsigned addr, uint8 data) {
  if(!Memory::debugger_access())
    cpu.synchronize_coprocessor();

  switch ((addr & 0x30) >> 4) {
  case 0: // serial controller (keyboard, RS-232)
    serial.write(addr & 3, data);
    break;

  case 1: // parallel controller (printer) (TODO)
    break;

  case 2: // floppy disk controller (TODO)
    if (data == 0x36)
      fdc_dio = false; // software reset
    else
      fdc_dio = true; // other commands
    break;
  
  case 3: // floppy disk controller, terminal count (TODO)
    break;
  }
}

static int scancode_table[] = {
//Escape, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
  0x00,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,  -1,
//PrintScreen, ScrollLock, Pause, Tilde,
  -1,          -1,         -1,    -1,
//Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
//Dash, Equal, Backspace,
  0x0b, 0x4d,  0x0e,
//Insert, Delete, Home, End, PageUp, PageDown,
  -1,     0x39,   -1,   -1,  -1,     -1,
//A,    B,    C,    D,    E,    F,    G,    H,    I,    J,    K,    L,    M,
  0x1d, 0x2d, 0x2b, 0x1f, 0x12, 0x20, 0x21, 0x22, 0x17, 0x23, 0x24, 0x25, 0x2f,
//N,    O,    P,    Q,    R,    S,    T,    U,    V,    W,    X,    Y,    Z,
  0x2e, 0x18, 0x19, 0x10, 0x13, 0x1e, 0x14, 0x16, 0x2c, 0x11, 0x2a, 0x15, 0x29,
//LeftBracket, RightBracket, Backslash, Semicolon, Apostrophe, Comma, Period, Slash,
//                                                 ^- (actually sends ':' here instead)
  0x1b,        0x28,         0x0d,      0x26,      0x27,       0x30,  0x31,   0x32,
//Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9, Keypad0,
  0x4a,    0x4b,    0x4c,    0x46,    0x47,    0x48,    0x42,    0x43,    0x44,    0x4e,
//Point, Enter, Add,  Subtract, Multiply, Divide,
  0x50,  0x1c,  0x49, 0x40,     0x45,     0x41,
//NumLock, CapsLock,
  -1,      -1,
//Up, Down, Left, Right,
  -1, -1,   -1,   -1,
//Tab,  Return, Spacebar, Menu,
  0x0f, 0x1c,   0x34,     -1,
//Shift, Control, Alt, Super,
  0x70,  0x74,    -1,  -1,
};

void DOS::send_key(unsigned scancode, bool on) {
  if (scancode < sizeof(scancode_table)
      && scancode_table[scancode] != -1) {
    // map application keystroke to PC-9801 keyboard scan code
    uint8 key = scancode_table[scancode];
    if (on) serial.send_data(0, key);
    else    serial.send_data(0, key | 0x80);
  }
}

#endif
