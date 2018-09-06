namespace Controllers {

void TurboInput::cache() {
  if(state) {
    cachedState = (counter < holdHi ? state : 0);
    if(++counter >= holdHi + holdLo) counter = 0;
  } else {
    cachedState = 0;
    counter = 0;
  }
}

TurboInput::TurboInput(const char *label, const char *configName) :
DigitalInput(label, configName) {
  holdHi = 2;
  holdLo = 2;
  counter = 0;
}

int16_t Gamepad::status(unsigned index, unsigned id) const {
  if(config().input.allowInvalidInput == false) {
    //block up+down and left+right combinations:
    //a real gamepad has a pivot in the D-pad that makes this impossible;
    //some software titles will crash if up+down or left+right are detected
    if(id == (unsigned)SNES::Input::JoypadID::Down && up.cachedState) return 0;
    if(id == (unsigned)SNES::Input::JoypadID::Right && left.cachedState) return 0;
  }

  switch((SNES::Input::JoypadID)id) {
    case SNES::Input::JoypadID::Up: return up.cachedState;
    case SNES::Input::JoypadID::Down: return down.cachedState;
    case SNES::Input::JoypadID::Left: return left.cachedState;
    case SNES::Input::JoypadID::Right: return right.cachedState;
    case SNES::Input::JoypadID::A: return a.cachedState | turboA.cachedState;
    case SNES::Input::JoypadID::B: return b.cachedState | turboB.cachedState;
    case SNES::Input::JoypadID::X: return x.cachedState | turboX.cachedState;
    case SNES::Input::JoypadID::Y: return y.cachedState | turboY.cachedState;
    case SNES::Input::JoypadID::L: return l.cachedState | turboL.cachedState;
    case SNES::Input::JoypadID::R: return r.cachedState | turboR.cachedState;
    case SNES::Input::JoypadID::Select: return select.cachedState;
    case SNES::Input::JoypadID::Start: return start.cachedState;
  }
  return 0;
}

Gamepad::Gamepad(unsigned category, const char *label, const char *configName) :
InputGroup(category, label),
up("Up", string() << "input." << configName << ".up"),
down("Down", string() << "input." << configName << ".down"),
left("Left", string() << "input." << configName << ".left"),
right("Right", string() << "input." << configName << ".right"),
b("B", string() << "input." << configName << ".b"),
a("A", string() << "input." << configName << ".a"),
y("Y", string() << "input." << configName << ".y"),
x("X", string() << "input." << configName << ".x"),
l("L", string() << "input." << configName << ".l"),
r("R", string() << "input." << configName << ".r"),
select("Select", string() << "input." << configName << ".select"),
start("Start", string() << "input." << configName << ".start"),
turboB("Turbo B", string() << "input." << configName << ".turboB"),
turboA("Turbo A", string() << "input." << configName << ".turboA"),
turboY("Turbo Y", string() << "input." << configName << ".turboY"),
turboX("Turbo X", string() << "input." << configName << ".turboX"),
turboL("Turbo L", string() << "input." << configName << ".turboL"),
turboR("Turbo R", string() << "input." << configName << ".turboR") {
  attach(&up); attach(&down); attach(&left); attach(&right);
  attach(&b); attach(&a); attach(&y); attach(&x);
  attach(&l); attach(&r); attach(&select); attach(&start);
  attach(&turboB); attach(&turboA); attach(&turboY); attach(&turboX);
  attach(&turboL); attach(&turboR);

  if(this == &gamepad1) {
    up.name = "KB0::Up";
    down.name = "KB0::Down";
    left.name = "KB0::Left";
    right.name = "KB0::Right";
    b.name = "KB0::Z";
    a.name = "KB0::X";
    y.name = "KB0::A";
    x.name = "KB0::S";
    l.name = "KB0::D";
    r.name = "KB0::C";
    select.name = "KB0::Apostrophe";
    start.name = "KB0::Return";
  }
}

//

int16_t Multitap::status(unsigned index, unsigned id) const {
  switch(index & 3) { default:
    case 0: return port1.status(index, id);
    case 1: return port2.status(index, id);
    case 2: return port3.status(index, id);
    case 3: return port4.status(index, id);
  }
}

Multitap::Multitap(Gamepad &port1_, Gamepad &port2_, Gamepad &port3_, Gamepad &port4_) :
InputGroup(InputCategory::Hidden, "Multitap"),
port1(port1_), port2(port2_), port3(port3_), port4(port4_) {
}

//

void AsciiSwitch::poll() {
  DigitalInput::poll();

  //only change state when controller is active
  if(!parent) return;
  if(parent->category == InputCategory::Port1 && mapper().port1 != parent) return;
  if(parent->category == InputCategory::Port2 && mapper().port2 != parent) return;

  if(previousState != state && state) {
    switch(mode) {
      case Off: mode = Turbo; utility.showMessage(string() << label << " set to turbo."); break;
      case Turbo: mode = Auto; utility.showMessage(string() << label << " set to auto."); break;
      case Auto: mode = Off; utility.showMessage(string() << label << " set to off."); break;
    }
  }
}

AsciiSwitch::AsciiSwitch(const char *label, const char *configName) :
DigitalInput(label, configName) {
  mode = Off;
}

void AsciiInput::cache() {
  if(asciiSwitch->mode == AsciiSwitch::Off) {
    cachedState = state;
  } else if(asciiSwitch->mode == AsciiSwitch::Turbo) {
    if(state) {
      cachedState = (counter < holdHi ? state : 0);
      if(++counter >= holdHi + holdLo) counter = 0;
    } else {
      cachedState = 0;
      counter = 0;
    }
  } else if(asciiSwitch->mode == AsciiSwitch::Auto) {
    cachedState = (counter < holdHi);
    if(++counter >= holdHi + holdLo) counter = 0;
  }
}

AsciiInput::AsciiInput(const char *label, const char *configName) :
DigitalInput(label, configName) {
  holdHi = 2;
  holdLo = 2;
  counter = 0;
}

void AsciiSlowMotion::poll() {
  DigitalInput::poll();

  //only change state when controller is active
  if(!parent) return;
  if(parent->category == InputCategory::Port1 && mapper().port1 != parent) return;
  if(parent->category == InputCategory::Port2 && mapper().port2 != parent) return;

  if(previousState != state && state) {
    if(enabled == false) {
      enabled = true;
      utility.showMessage(string() << label << " enabled.");
    } else {
      enabled = false;
      utility.showMessage(string() << label << " disabled.");
    }
  }
}

void AsciiSlowMotion::cache() {
  if(enabled == false) {
    cachedState = 0;
  } else {
    cachedState = counter < holdHi;
    if(++counter >= holdHi + holdLo) counter = 0;
  }
}

AsciiSlowMotion::AsciiSlowMotion(const char *label, const char *configName) :
DigitalInput(label, configName) {
  enabled = false;
  holdHi = 2;
  holdLo = 2;
}

int16_t Asciipad::status(unsigned index, unsigned id) const {
  if(config().input.allowInvalidInput == false) {
    if(id == (unsigned)SNES::Input::JoypadID::Down && up.cachedState) return 0;
    if(id == (unsigned)SNES::Input::JoypadID::Right && left.cachedState) return 0;
  }

  switch((SNES::Input::JoypadID)id) {
    case SNES::Input::JoypadID::Up: return up.cachedState;
    case SNES::Input::JoypadID::Down: return down.cachedState;
    case SNES::Input::JoypadID::Left: return left.cachedState;
    case SNES::Input::JoypadID::Right: return right.cachedState;
    case SNES::Input::JoypadID::A: return a.cachedState;
    case SNES::Input::JoypadID::B: return b.cachedState;
    case SNES::Input::JoypadID::X: return x.cachedState;
    case SNES::Input::JoypadID::Y: return y.cachedState;
    case SNES::Input::JoypadID::L: return l.cachedState;
    case SNES::Input::JoypadID::R: return r.cachedState;
    case SNES::Input::JoypadID::Select: return select.cachedState;
    case SNES::Input::JoypadID::Start: return start.cachedState | slowMotion.cachedState;
  }
  return 0;
}

Asciipad::Asciipad(unsigned category, const char *label, const char *configName) :
InputGroup(category, label),
up("Up", string() << "input." << configName << ".up"),
down("Down", string() << "input." << configName << ".down"),
left("Left", string() << "input." << configName << ".left"),
right("Right", string() << "input." << configName << ".right"),
b("B", string() << "input." << configName << ".b"),
a("A", string() << "input." << configName << ".a"),
y("Y", string() << "input." << configName << ".y"),
x("X", string() << "input." << configName << ".x"),
l("L", string() << "input." << configName << ".l"),
r("R", string() << "input." << configName << ".r"),
select("Select", string() << "input." << configName << ".select"),
start("Start", string() << "input." << configName << ".start"),
switchB("B Switch", string() << "input." << configName << ".bSwitch"),
switchA("A Switch", string() << "input." << configName << ".aSwitch"),
switchY("Y Switch", string() << "input." << configName << ".ySwitch"),
switchX("X Switch", string() << "input." << configName << ".xSwitch"),
switchL("L Switch", string() << "input." << configName << ".lSwitch"),
switchR("R Switch", string() << "input." << configName << ".rSwitch"),
slowMotion("Slow Motion", string() << "input." << configName << ".slowMotion") {
  b.asciiSwitch = &switchB;
  a.asciiSwitch = &switchA;
  y.asciiSwitch = &switchY;
  x.asciiSwitch = &switchX;
  l.asciiSwitch = &switchL;
  r.asciiSwitch = &switchR;

  attach(&up); attach(&down); attach(&left); attach(&right);
  attach(&b); attach(&a); attach(&y); attach(&x);
  attach(&l); attach(&r); attach(&select); attach(&start);
  attach(&switchB); attach(&switchA); attach(&switchY); attach(&switchX);
  attach(&switchL); attach(&switchR); attach(&slowMotion);

  if(this == &asciipad1) {
    up.name = "KB0::Up";
    down.name = "KB0::Down";
    left.name = "KB0::Left";
    right.name = "KB0::Right";
    b.name = "KB0::Z";
    a.name = "KB0::X";
    y.name = "KB0::A";
    x.name = "KB0::S";
    l.name = "KB0::D";
    r.name = "KB0::C";
    select.name = "KB0::Apostrophe";
    start.name = "KB0::Return";
  }
}

//

int16_t Mouse::status(unsigned index, unsigned id) const {
  switch((SNES::Input::MouseID)id) {
    case SNES::Input::MouseID::X: return x.cachedState;
    case SNES::Input::MouseID::Y: return y.cachedState;
    case SNES::Input::MouseID::Left: return left.cachedState;
    case SNES::Input::MouseID::Right: return right.cachedState;
  }
  return 0;
}

Mouse::Mouse(unsigned category, const char *label, const char *configName) :
InputGroup(category, label),
x("X-axis", string() << "input." << configName << ".x"),
y("Y-axis", string() << "input." << configName << ".y"),
left("Left Button", string() << "input." << configName << ".left"),
right("Right Button", string() << "input." << configName << ".right") {
  attach(&x); attach(&y); attach(&left); attach(&right);

  x.name = "MS0::Xaxis";
  y.name = "MS0::Yaxis";
  left.name = "MS0::Button0";
  right.name = "MS0::Button2";
}

//

int16_t SuperScope::status(unsigned index, unsigned id) const {
  switch((SNES::Input::SuperScopeID)id) {
    case SNES::Input::SuperScopeID::X: return x.cachedState;
    case SNES::Input::SuperScopeID::Y: return y.cachedState;
    case SNES::Input::SuperScopeID::Trigger: return trigger.cachedState;
    case SNES::Input::SuperScopeID::Cursor: return cursor.cachedState;
    case SNES::Input::SuperScopeID::Turbo: return turbo.cachedState;
    case SNES::Input::SuperScopeID::Pause: return pause.cachedState;
  }
  return 0;
}

SuperScope::SuperScope(unsigned category, const char *label, const char *configName) :
InputGroup(category, label),
x("X-axis", string() << "input." << configName << ".x"),
y("Y-axis", string() << "input." << configName << ".y"),
trigger("Trigger", string() << "input." << configName << ".trigger"),
cursor("Cursor", string() << "input." << configName << ".cursor"),
turbo("Turbo", string() << "input." << configName << ".turbo"),
pause("Pause", string() << "input." << configName << ".pause") {
  attach(&x); attach(&y); attach(&trigger); attach(&cursor);
  attach(&turbo); attach(&pause);

  x.name = "MS0::Xaxis";
  y.name = "MS0::Yaxis";
  trigger.name = "MS0::Button0";
  cursor.name = "MS0::Button2";
  turbo.name = "KB0::T";
  pause.name = "KB0::P";
}

//

int16_t Justifier::status(unsigned index, unsigned id) const {
  switch((SNES::Input::JustifierID)id) {
    case SNES::Input::JustifierID::X: return x.cachedState;
    case SNES::Input::JustifierID::Y: return y.cachedState;
    case SNES::Input::JustifierID::Trigger: return trigger.cachedState;
    case SNES::Input::JustifierID::Start: return start.cachedState;
  }
  return 0;
}

Justifier::Justifier(unsigned category, const char *label, const char *configName) :
InputGroup(category, label),
x("X-axis", string() << "input." << configName << ".x"),
y("Y-axis", string() << "input." << configName << ".y"),
trigger("Trigger", string() << "input." << configName << ".trigger"),
start("Start", string() << "input." << configName << ".start") {
  attach(&x); attach(&y); attach(&trigger); attach(&start);

  if(this == &justifier1) {
    x.name = "MS0::Xaxis";
    y.name = "MS0::Yaxis";
    trigger.name = "MS0::Button0";
    start.name = "MS0::Button2";
  }
}

//

int16_t Justifiers::status(unsigned index, unsigned id) const {
  switch(index & 1) { default:
    case 0: return port1.status(index, id);
    case 1: return port2.status(index, id);
  }
}

Justifiers::Justifiers(Justifier &port1_, Justifier &port2_) :
InputGroup(InputCategory::Hidden, "Justifiers"),
port1(port1_), port2(port2_) {
}

//

int16_t NTTDataKeypad::status(unsigned index, unsigned id) const {
  if(config().input.allowInvalidInput == false) {
    //block up+down and left+right combinations:
    //a real gamepad has a pivot in the D-pad that makes this impossible;
    //some software titles will crash if up+down or left+right are detected
    if(id == (unsigned)SNES::Input::NTTDataKeypadID::Down && up.cachedState) return 0;
    if(id == (unsigned)SNES::Input::NTTDataKeypadID::Right && left.cachedState) return 0;
  }

  switch((SNES::Input::NTTDataKeypadID)id) {
    case SNES::Input::NTTDataKeypadID::Up: return up.cachedState;
    case SNES::Input::NTTDataKeypadID::Down: return down.cachedState;
    case SNES::Input::NTTDataKeypadID::Left: return left.cachedState;
    case SNES::Input::NTTDataKeypadID::Right: return right.cachedState;
    case SNES::Input::NTTDataKeypadID::A: return a.cachedState | turboA.cachedState;
    case SNES::Input::NTTDataKeypadID::B: return b.cachedState | turboB.cachedState;
    case SNES::Input::NTTDataKeypadID::X: return x.cachedState | turboX.cachedState;
    case SNES::Input::NTTDataKeypadID::Y: return y.cachedState | turboY.cachedState;
    case SNES::Input::NTTDataKeypadID::L: return l.cachedState | turboL.cachedState;
    case SNES::Input::NTTDataKeypadID::R: return r.cachedState | turboR.cachedState;
    case SNES::Input::NTTDataKeypadID::Select: return select.cachedState;
    case SNES::Input::NTTDataKeypadID::Start: return start.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit0: return digit0.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit1: return digit1.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit2: return digit2.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit3: return digit3.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit4: return digit4.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit5: return digit5.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit6: return digit6.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit7: return digit7.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit8: return digit8.cachedState;
    case SNES::Input::NTTDataKeypadID::Digit9: return digit9.cachedState;
    case SNES::Input::NTTDataKeypadID::Star: return star.cachedState;
    case SNES::Input::NTTDataKeypadID::Hash: return hash.cachedState;
    case SNES::Input::NTTDataKeypadID::Period: return period.cachedState;
    case SNES::Input::NTTDataKeypadID::C: return c.cachedState;
    case SNES::Input::NTTDataKeypadID::Hangup: return hangup.cachedState;
  }
  return 0;
}


NTTDataKeypad::NTTDataKeypad(unsigned category, const char *label, const char *configName) :
InputGroup(category, label),
up("Up", string() << "input." << configName << ".up"),
down("Down", string() << "input." << configName << ".down"),
left("Left", string() << "input." << configName << ".left"),
right("Right", string() << "input." << configName << ".right"),
b("B", string() << "input." << configName << ".b"),
a("A", string() << "input." << configName << ".a"),
y("Y", string() << "input." << configName << ".y"),
x("X", string() << "input." << configName << ".x"),
l("L", string() << "input." << configName << ".l"),
r("R", string() << "input." << configName << ".r"),
select("Select", string() << "input." << configName << ".select"),
start("Start", string() << "input." << configName << ".start"),
digit0("0", string() << "input." << configName << ".0"),
digit1("1", string() << "input." << configName << ".1"),
digit2("2", string() << "input." << configName << ".2"),
digit3("3", string() << "input." << configName << ".3"),
digit4("4", string() << "input." << configName << ".4"),
digit5("5", string() << "input." << configName << ".5"),
digit6("6", string() << "input." << configName << ".6"),
digit7("7", string() << "input." << configName << ".7"),
digit8("8", string() << "input." << configName << ".8"),
digit9("9", string() << "input." << configName << ".9"),
star("*", string() << "input." << configName << ".star"),
hash("#", string() << "input." << configName << ".hash"),
period(".", string() << "input." << configName << ".period"),
c("C", string() << "input." << configName << ".c"),
hangup("Hang up", string() << "input." << configName << ".hangup"),
turboB("Turbo B", string() << "input." << configName << ".turboB"),
turboA("Turbo A", string() << "input." << configName << ".turboA"),
turboY("Turbo Y", string() << "input." << configName << ".turboY"),
turboX("Turbo X", string() << "input." << configName << ".turboX"),
turboL("Turbo L", string() << "input." << configName << ".turboL"),
turboR("Turbo R", string() << "input." << configName << ".turboR") {
  attach(&up); attach(&down); attach(&left); attach(&right);
  attach(&b); attach(&a); attach(&y); attach(&x);
  attach(&l); attach(&r); attach(&select); attach(&start);
  attach(&digit0); attach(&digit1); attach(&digit2); attach(&digit3);
  attach(&digit4); attach(&digit5); attach(&digit6); attach(&digit7);
  attach(&digit8); attach(&digit9); attach(&star); attach(&hash);
  attach(&period); attach(&c); attach(&hangup);
  attach(&turboB); attach(&turboA); attach(&turboY); attach(&turboX);
  attach(&turboL); attach(&turboR);

  if(this == &nttdatakeypad1) {
    up.name = "KB0::Up";
    down.name = "KB0::Down";
    left.name = "KB0::Left";
    right.name = "KB0::Right";
    b.name = "KB0::Z";
    a.name = "KB0::X";
    y.name = "KB0::A";
    x.name = "KB0::S";
    l.name = "KB0::D";
    r.name = "KB0::C";
    select.name = "KB0::Apostrophe";
    start.name = "KB0::Return";
    digit0.name = "KB0::Num0";
    digit1.name = "KB0::Num1";
    digit2.name = "KB0::Num2";
    digit3.name = "KB0::Num3";
    digit4.name = "KB0::Num4";
    digit5.name = "KB0::Num5";
    digit6.name = "KB0::Num6";
    digit7.name = "KB0::Num7";
    digit8.name = "KB0::Num8";
    digit9.name = "KB0::Num9";
    star.name = "KB0::Multiply";
    hash.name = "KB0::N";
    period.name = "KB0::Period";
    c.name = "KB0::Backspace";
    hangup.name = "KB0::End";
  }
}

//

Gamepad gamepad1(InputCategory::Port1, "Gamepad", "gamepad1");
Asciipad asciipad1(InputCategory::Port1, "asciiPad", "asciipad1");
Gamepad multitap1a(InputCategory::Port1, "Multitap - Port 1", "multitap1a");
Gamepad multitap1b(InputCategory::Port1, "Multitap - Port 2", "multitap1b");
Gamepad multitap1c(InputCategory::Port1, "Multitap - Port 3", "multitap1c");
Gamepad multitap1d(InputCategory::Port1, "Multitap - Port 4", "multitap1d");
Multitap multitap1(multitap1a, multitap1b, multitap1c, multitap1d);
Mouse mouse1(InputCategory::Port1, "Mouse", "mouse1");
NTTDataKeypad nttdatakeypad1(InputCategory::Port1, "NTT Data Keypad", "nttdatakeypad1");

Gamepad gamepad2(InputCategory::Port2, "Gamepad", "gamepad2");
Asciipad asciipad2(InputCategory::Port2, "asciiPad", "asciipad2");
Gamepad multitap2a(InputCategory::Port2, "Multitap - Port 1", "multitap2a");
Gamepad multitap2b(InputCategory::Port2, "Multitap - Port 2", "multitap2b");
Gamepad multitap2c(InputCategory::Port2, "Multitap - Port 3", "multitap2c");
Gamepad multitap2d(InputCategory::Port2, "Multitap - Port 4", "multitap2d");
Multitap multitap2(multitap2a, multitap2b, multitap2c, multitap2d);
Mouse mouse2(InputCategory::Port2, "Mouse", "mouse2");
SuperScope superscope(InputCategory::Port2, "Super Scope", "superscope");
Justifier justifier1(InputCategory::Port2, "Justifier 1", "justifier1");
Justifier justifier2(InputCategory::Port2, "Justifier 2", "justifier2");
Justifiers justifiers(justifier1, justifier2);


}
