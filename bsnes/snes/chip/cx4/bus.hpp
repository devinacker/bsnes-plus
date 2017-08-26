struct Cx4Bus : Bus {
  void init();
};

struct UnmappedCx4 : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
};

struct Cx4ROM : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
};

struct Cx4RAM : Memory {
  unsigned size() const;
  alwaysinline uint8 read(unsigned);
  alwaysinline void write(unsigned, uint8);
};

namespace memory {
  extern Cx4ROM cx4rom;
  extern Cx4RAM cx4ram;
}
