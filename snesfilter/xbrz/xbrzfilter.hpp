#include <cstdint>

class XbrzFilter {
public:
  XbrzFilter(size_t factor): factor(factor) {}

public:
  void size(unsigned&, unsigned&, unsigned, unsigned);
  void render(uint32_t*, unsigned, const uint16_t*, unsigned, unsigned, unsigned);

private:
  size_t factor;
} filter_xbrz2x(2), filter_xbrz3x(3), filter_xbrz4x(4), filter_xbrz5x(5);
