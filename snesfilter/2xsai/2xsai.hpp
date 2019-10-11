class _2xSaIFilter {
public:
  void size(unsigned&, unsigned&, unsigned, unsigned);
  void render(uint32_t*, unsigned, const uint16_t*, unsigned, unsigned, unsigned);

  _2xSaIFilter();
  ~_2xSaIFilter();

private:
  uint32_t *temp;
  unsigned _width, _height;
} filter_2xsai;

class Super2xSaIFilter {
public:
  void size(unsigned&, unsigned&, unsigned, unsigned);
  void render(uint32_t*, unsigned, const uint16_t*, unsigned, unsigned, unsigned);

  Super2xSaIFilter();
  ~Super2xSaIFilter();

private:
  uint32_t *temp;
  unsigned _width, _height;
} filter_super2xsai;

class SuperEagleFilter {
public:
  void size(unsigned&, unsigned&, unsigned, unsigned);
  void render(uint32_t*, unsigned, const uint16_t*, unsigned, unsigned, unsigned);

  SuperEagleFilter();
  ~SuperEagleFilter();

private:
  uint32_t *temp;
  unsigned _width, _height;
} filter_supereagle;
