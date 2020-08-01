#include <snes.hpp>

#define DOS_CPP
namespace SNES {
  #include "dos_base.cpp"
  #include "dos_serial.cpp"
  #include "dos_floppy.cpp"
  #include "serialization.cpp"
}
