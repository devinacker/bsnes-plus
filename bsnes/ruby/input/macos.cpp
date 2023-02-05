//==================
//macOS input driver
//==================
//Original (Carbon) implementation by byuu
//Mouse and gamepad (IOKit/IOHIDLib) additions by optiroc@gmail.com
//
//Techically keyboard and mouse could also use IOHIDLib, but as long
//as Carbon is still available there's no good reason to pull it out.

#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>

namespace ruby {

static void CI_hid_device_added_cb(void *p, IOReturn result, void *sender, IOHIDDeviceRef device_ref);
static void CI_hid_device_removed_cb(void *p, IOReturn result, void *sender, IOHIDDeviceRef device_ref);
static void CI_hid_input_value_cb(void *p, IOReturn result, void *sender, IOHIDValueRef value_ref);

static uint32_t CI_hid_usage_pairs[]  = {
  kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick,
  kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad
};
static int CI_hid_usage_pairs_len = 2;

class pInputmacOS {
  int16_t table_buffer[Scancode::Limit];

public:
  struct {
    bool mouse_acquired;
    IOHIDManagerRef hid_manager_ref = NULL;
    IOHIDDeviceRef hid_joypads[Joypad::Count];
  } device;

  struct {
    uintptr_t handle;
  } settings;

  pInputmacOS() {
    settings.handle = 0;
    memset(table_buffer, 0, Scancode::Limit * sizeof(int16_t));
  }

  ~pInputmacOS() {
    term();
  }

  bool init() {
    init_hid_manager();
    return true;
  }

  void term() {
    unacquire();
    term_hid_manager();
  }

  bool cap(const string& name) {
    if(name == Input::Handle) return true;
    if(name == Input::KeyboardSupport) return true;
    if(name == Input::MouseSupport) return true;
    if(name == Input::JoypadSupport) return true;
    return false;
  }

  any get(const string& name) {
    if(name == Input::Handle) return (uintptr_t)settings.handle;
    return false;
  }

  bool set(const string& name, const any &value) {
    if(name == Input::Handle) {
      settings.handle = any_cast<uintptr_t>(value);
      return true;
    }
    return false;
  }

  bool acquire() {
    if(acquired()) return true;

    CGDisplayHideCursor(0);
    CGAssociateMouseAndMouseCursorPosition(false);
    return device.mouse_acquired = true;
  }

  bool unacquire() {
    if(acquired()) {
      CGDisplayShowCursor(0);
      CGAssociateMouseAndMouseCursorPosition(true);
      device.mouse_acquired = false;
    }
    return true;
  }

  bool acquired() {
    return device.mouse_acquired;
  }

  bool poll(int16_t *table) {
    memcpy(table, table_buffer, Scancode::Limit * sizeof(int16_t));

    //========
    //Keyboard
    //========

    KeyMap keys;
    GetKeys(keys);
    uint8_t *keymap = (uint8_t*)keys;

    #define map(id, name) table[keyboard(0)[name]] = (bool)(keymap[id >> 3] & (1 << (id & 7)))
    map(0x35, Keyboard::Escape);

    map(0x7a, Keyboard::F1);
    map(0x78, Keyboard::F2);
    map(0x63, Keyboard::F3);
    map(0x76, Keyboard::F4);
    map(0x60, Keyboard::F5);
    map(0x61, Keyboard::F6);
    map(0x62, Keyboard::F7);
    map(0x64, Keyboard::F8);
    map(0x65, Keyboard::F9);
    map(0x6d, Keyboard::F10);
    map(0x67, Keyboard::F11);
  //map(0x??, Keyboard::F12);

    map(0x69, Keyboard::PrintScreen);
  //map(0x??, Keyboard::ScrollLock);
    map(0x71, Keyboard::Pause);

    map(0x32, Keyboard::Tilde);
    map(0x12, Keyboard::Num1);
    map(0x13, Keyboard::Num2);
    map(0x14, Keyboard::Num3);
    map(0x15, Keyboard::Num4);
    map(0x17, Keyboard::Num5);
    map(0x16, Keyboard::Num6);
    map(0x1a, Keyboard::Num7);
    map(0x1c, Keyboard::Num8);
    map(0x19, Keyboard::Num9);
    map(0x1d, Keyboard::Num0);

    map(0x1b, Keyboard::Dash);
    map(0x18, Keyboard::Equal);
    map(0x33, Keyboard::Backspace);

    map(0x72, Keyboard::Insert);
    map(0x75, Keyboard::Delete);
    map(0x73, Keyboard::Home);
    map(0x77, Keyboard::End);
    map(0x74, Keyboard::PageUp);
    map(0x79, Keyboard::PageDown);

    map(0x00, Keyboard::A);
    map(0x0b, Keyboard::B);
    map(0x08, Keyboard::C);
    map(0x02, Keyboard::D);
    map(0x0e, Keyboard::E);
    map(0x03, Keyboard::F);
    map(0x05, Keyboard::G);
    map(0x04, Keyboard::H);
    map(0x22, Keyboard::I);
    map(0x26, Keyboard::J);
    map(0x28, Keyboard::K);
    map(0x25, Keyboard::L);
    map(0x2e, Keyboard::M);
    map(0x2d, Keyboard::N);
    map(0x1f, Keyboard::O);
    map(0x23, Keyboard::P);
    map(0x0c, Keyboard::Q);
    map(0x0f, Keyboard::R);
    map(0x01, Keyboard::S);
    map(0x11, Keyboard::T);
    map(0x20, Keyboard::U);
    map(0x09, Keyboard::V);
    map(0x0d, Keyboard::W);
    map(0x07, Keyboard::X);
    map(0x10, Keyboard::Y);
    map(0x06, Keyboard::Z);

    map(0x21, Keyboard::LeftBracket);
    map(0x1e, Keyboard::RightBracket);
    map(0x2a, Keyboard::Backslash);
    map(0x29, Keyboard::Semicolon);
    map(0x27, Keyboard::Apostrophe);
    map(0x2b, Keyboard::Comma);
    map(0x2f, Keyboard::Period);
    map(0x2c, Keyboard::Slash);

    map(0x53, Keyboard::Keypad1);
    map(0x54, Keyboard::Keypad2);
    map(0x55, Keyboard::Keypad3);
    map(0x56, Keyboard::Keypad4);
    map(0x57, Keyboard::Keypad5);
    map(0x58, Keyboard::Keypad6);
    map(0x59, Keyboard::Keypad7);
    map(0x5b, Keyboard::Keypad8);
    map(0x5c, Keyboard::Keypad9);
    map(0x52, Keyboard::Keypad0);

  //map(0x??, Keyboard::Point);
    map(0x4c, Keyboard::Enter);
    map(0x45, Keyboard::Add);
    map(0x4e, Keyboard::Subtract);
    map(0x43, Keyboard::Multiply);
    map(0x4b, Keyboard::Divide);

    map(0x47, Keyboard::NumLock);
  //map(0x39, Keyboard::CapsLock);

    map(0x7e, Keyboard::Up);
    map(0x7d, Keyboard::Down);
    map(0x7b, Keyboard::Left);
    map(0x7c, Keyboard::Right);

    map(0x30, Keyboard::Tab);
    map(0x24, Keyboard::Return);
    map(0x31, Keyboard::Spacebar);
  //map(0x??, Keyboard::Menu);

    map(0x38, Keyboard::Shift);
    map(0x3b, Keyboard::Control);
    map(0x3a, Keyboard::Alt);
    map(0x37, Keyboard::Super);
    #undef map

    //=====
    //Mouse
    //=====

    if (acquired()) {
      int32_t delta_x, delta_y;
      CGGetLastMouseDelta(&delta_x, &delta_y);
      uint32 button_state = GetCurrentEventButtonState();

      for (unsigned i = 0; i < (unsigned)Mouse::Count; ++i) {
        table[mouse(i).axis(0)] = delta_x;
        table[mouse(i).axis(1)] = delta_y;
        table[mouse(i).axis(2)] = 0;

        table[mouse(i).button(0)] = (bool)(button_state & 1);
        for(unsigned n = 1; n < Mouse::Buttons; ++n) table[mouse(i).button(n)] = (bool)(button_state & 2);
      }
    }

    return true;
  }


  //===============
  //IOHIDLib access
  //===============

  void init_hid_manager() {
    // Create HID manager and register callbacks on current thread

    if (device.hid_manager_ref) return;
    device.hid_manager_ref = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    if (device.hid_manager_ref) {
      IOHIDManagerRegisterDeviceMatchingCallback(device.hid_manager_ref, &CI_hid_device_added_cb, (void *)this);
      IOHIDManagerRegisterDeviceRemovalCallback(device.hid_manager_ref, &CI_hid_device_removed_cb, (void *)this);
      IOHIDManagerRegisterInputValueCallback(device.hid_manager_ref, &CI_hid_input_value_cb, (void *)this);
      IOHIDManagerScheduleWithRunLoop(device.hid_manager_ref, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

      CFMutableArrayRef matches = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

      if (matches) {
        for (int i = 0; i < CI_hid_usage_pairs_len; ++i) {
          CFMutableDictionaryRef match = create_matching_dict(CI_hid_usage_pairs[i << 1], CI_hid_usage_pairs[(i << 1) + 1]);
          if (match) {
            CFArrayAppendValue(matches, (void *)match);
            CFRelease(match);
          }
        }

        IOHIDManagerSetDeviceMatchingMultiple(device.hid_manager_ref, matches);
        CFRelease(matches);

        IOHIDManagerOpen(device.hid_manager_ref, kIOHIDOptionsTypeNone);
      }
    }
  }

  void term_hid_manager() {
    if (device.hid_manager_ref) {
      IOHIDManagerRegisterInputValueCallback(device.hid_manager_ref, NULL, NULL);
      IOHIDManagerRegisterDeviceRemovalCallback(device.hid_manager_ref, NULL, NULL);
      IOHIDManagerRegisterDeviceMatchingCallback(device.hid_manager_ref, NULL, NULL);
      IOHIDManagerUnscheduleFromRunLoop(device.hid_manager_ref, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
      IOHIDManagerClose(device.hid_manager_ref, 0);
      device.hid_manager_ref = NULL;
    }
  }

  int hid_get_device_slot(IOHIDDeviceRef device_ref) {
    for(unsigned i = 0; i < Joypad::Count; ++i) {
      if (device.hid_joypads[i] == device_ref) return i;
    }
    return -1;
  }

  void hid_register_device(IOHIDDeviceRef device_ref) {
    if (hid_get_device_slot(device_ref) != -1) return;

    for (unsigned i = 0; i < Joypad::Count; ++i) {
      if (!device.hid_joypads[i]) {
        //printf("hid_register_device: slot %d\n", i);
        device.hid_joypads[i] = device_ref;
        break;
      }
    }
  }

  void hid_unregister_device(IOHIDDeviceRef device_ref) {
    int device_slot = hid_get_device_slot(device_ref);
    if (device_slot != -1) {
      //printf("hid_unregister_device: slot %d\n", device_slot);
      device.hid_joypads[device_slot] = NULL;
      memset((void *)(table_buffer + joypad(device_slot).hat(0)), 0, Joypad::Scancode::Limit * sizeof(int16_t));
    }
  }

  void hid_input_value_received(IOHIDValueRef value_ref) {
    IOHIDElementRef element_ref = IOHIDValueGetElement(value_ref);
    if (CFGetTypeID(element_ref) != IOHIDElementGetTypeID()) return;

    IOHIDDeviceRef device_ref = IOHIDElementGetDevice(element_ref);
    int device_slot = hid_get_device_slot(device_ref);
    if (device_slot == -1) return;

    int value = IOHIDValueGetIntegerValue(value_ref);
    uint32_t usage = IOHIDElementGetUsage(element_ref);
    IOHIDElementType type = IOHIDElementGetType(element_ref);
    IOHIDElementCookie cookie = IOHIDElementGetCookie(element_ref);

    // Type: Button
    if (type == kIOHIDElementTypeInput_Button) {
      table_buffer[joypad(device_slot).button(cookie)] = (bool)value;
      //printf("hid_input_value_received - slot %d - button #%d %d\n", device_slot, cookie, value);
    }

    // Type: Axis (nop, implement when a suitable device is encountered)
    else if (type == kIOHIDElementTypeInput_Axis) {
      printf("hid_input_value_received - slot %d - axis - usage 0x%04x %d %d\n", device_slot, usage, value, cookie);
    }

    // Type: Misc
    else if (type == kIOHIDElementTypeInput_Misc) {

      // Type: Misc - Usage: Hat switch
      if (usage == kHIDUsage_GD_Hatswitch) {
        int min = (int)IOHIDElementGetLogicalMin(element_ref);
        int max = (int)IOHIDElementGetLogicalMax(element_ref);
        int range = max - min;
        if (range == 3) value *= 2;
        value -= min;
        switch (value) {
          case 0: table_buffer[joypad(device_slot).hat(0)] = Joypad::HatUp; break;
          case 1: table_buffer[joypad(device_slot).hat(0)] = Joypad::HatUp | Joypad::HatRight; break;
          case 2: table_buffer[joypad(device_slot).hat(0)] = Joypad::HatRight; break;
          case 3: table_buffer[joypad(device_slot).hat(0)] = Joypad::HatDown | Joypad::HatRight; break;
          case 4: table_buffer[joypad(device_slot).hat(0)] = Joypad::HatDown; break;
          case 5: table_buffer[joypad(device_slot).hat(0)] = Joypad::HatDown | Joypad::HatLeft; break;
          case 6: table_buffer[joypad(device_slot).hat(0)] = Joypad::HatLeft; break;
          case 7: table_buffer[joypad(device_slot).hat(0)] = Joypad::HatUp | Joypad::HatLeft; break;
          default:
            table_buffer[joypad(device_slot).hat(0)] = Joypad::HatCenter; break;
        }
        //printf("hid_input_value_received - slot %d - hat %d (range %d...%d) %d\n", device_slot, value, min, max, cookie);
      }

      // Type: Misc - Usage: D-Pad
      else if (usage_is_dpad(usage)) {
        int16_t direction = Joypad::HatCenter;
        if (usage == kHIDUsage_GD_DPadUp && (bool)value) direction |= Joypad::HatUp;
        if (usage == kHIDUsage_GD_DPadDown && (bool)value) direction |= Joypad::HatDown;
        if (usage == kHIDUsage_GD_DPadRight && (bool)value) direction |= Joypad::HatRight;
        if (usage == kHIDUsage_GD_DPadLeft && (bool)value) direction |= Joypad::HatLeft;
        table_buffer[joypad(device_slot).hat(1)] = direction;
      }

      // Type: Misc - Usage: Not hat/d-pad
      else {

        // Type: Misc - Usage: Axis
        int axis = axis_index_from_usage(usage);
        if (axis != -1) {
          int scaled_value = IOHIDValueGetScaledValue(value_ref, kIOHIDValueScaleTypePhysical);
          int min = IOHIDElementGetPhysicalMin(element_ref);
          int max = IOHIDElementGetPhysicalMax(element_ref);
          scaled_value = (((scaled_value - min) * (INT16_MAX - INT16_MIN)) / (max - min)) + INT16_MIN;
          if (abs(scaled_value) < 1024) scaled_value = 0;
          table_buffer[joypad(device_slot).axis(axis)] = (int16_t)scaled_value;
        }

        // Drill down further when need arises
        else {
          printf("hid_input_value_received - slot %d - usage 0x%04x %d %d\n", device_slot, usage, value, cookie);
        }

      }
    }
  }

  int axis_index_from_usage(uint32_t usage) {
    switch (usage) {
      case kHIDUsage_GD_X: return 0;
      case kHIDUsage_GD_Y: return 1;
      case kHIDUsage_GD_Z: return 2;
      case kHIDUsage_GD_Rx: return 3;
      case kHIDUsage_GD_Ry: return 4;
      case kHIDUsage_GD_Rz: return 5;
      case kHIDUsage_GD_Vx: return 6;
      case kHIDUsage_GD_Vy: return 7;
      case kHIDUsage_GD_Vz: return 8;
      case kHIDUsage_GD_Vbrx: return 9;
      case kHIDUsage_GD_Vbry: return 10;
      case kHIDUsage_GD_Vbrz: return 11;

      default:
        return -1;
    }
  }

  bool usage_is_dpad(uint32_t usage) {
    return usage == kHIDUsage_GD_DPadUp || usage == kHIDUsage_GD_DPadDown ||
           usage == kHIDUsage_GD_DPadLeft || usage == kHIDUsage_GD_DPadRight;
  }

  CFMutableDictionaryRef create_matching_dict(uint32_t usage_page, uint32_t	usage) {
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                            &kCFTypeDictionaryKeyCallBacks,
                                                            &kCFTypeDictionaryValueCallBacks);
    CFNumberRef usage_page_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usage_page);
    CFNumberRef usage_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usage);

    if (dict && usage_page_ref && usage_ref) {
      CFDictionarySetValue(dict, CFSTR(kIOHIDPrimaryUsagePageKey), usage_page_ref);
      CFDictionarySetValue(dict, CFSTR(kIOHIDPrimaryUsageKey), usage_ref);
    }

    CFRelease(usage_ref);
    CFRelease(usage_page_ref);
    return dict;
  }

};

static void CI_hid_device_added_cb(void *p, IOReturn result, void *sender, IOHIDDeviceRef device_ref) {
  if (result == kIOReturnSuccess) {
    ((pInputmacOS*)p)->hid_register_device(device_ref);
  }
}

static void CI_hid_device_removed_cb(void *p, IOReturn result, void *sender, IOHIDDeviceRef device_ref) {
  ((pInputmacOS*)p)->hid_unregister_device(device_ref);
}

static void CI_hid_input_value_cb(void *p, IOReturn result, void *sender, IOHIDValueRef value_ref) {
  if (result == kIOReturnSuccess) {
    ((pInputmacOS*)p)->hid_input_value_received(value_ref);
  }
}

DeclareInput(macOS)

};
