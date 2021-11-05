class Interface;

class System : property<System> {
public:
  enum class Region : unsigned { NTSC = 0, PAL = 1, Autodetect = 2 };
  enum class ExpansionPortDevice : unsigned { None = 0, BSX = 1 };

  void run();
  void runtosave();

  void init(Interface*);
  void term();
  void power();
  void reset();
  void unload();

  void frame();
  void scanline();

  //return *active* system information (settings are cached upon power-on)
  readonly<Region> region;
  readonly<ExpansionPortDevice> expansion;
  readonly<unsigned> cpu_frequency;
  readonly<unsigned> apu_frequency;
  readonly<unsigned> serialize_size;

  serializer serialize();
  bool unserialize(serializer&);

  System();

private:
  Interface *intf;
  bool runthreadtosave(cothread_t&);

  void serialize(serializer&);
  void serialize_all(serializer&);
  void serialize_init();

  friend class Cartridge;
  friend class Video;
  friend class Audio;
  friend class Input;
};

struct Random {
  void seed(unsigned seed);
  unsigned operator()(unsigned result);
  void serialize(serializer&);
  
private:
  nall::random_cyclic _random;
};

#include <video/video.hpp>
#include <audio/audio.hpp>
#include <input/input.hpp>

#include <config/config.hpp>
#include <debugger/debugger.hpp>
#include <interface/interface.hpp>
#include <scheduler/scheduler.hpp>

extern System system;
extern Random random;
