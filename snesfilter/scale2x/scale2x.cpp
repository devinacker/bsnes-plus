#include "scale2x.hpp"

void Scale2xFilter::size(unsigned &outwidth, unsigned &outheight, unsigned width, unsigned height) {
  outwidth  = (width  <= 256) ? width  * 2 : width;
  outheight = (height <= 240) ? height * 2 : height;
}

void Scale2xFilter::render(
  uint32_t *output, unsigned outpitch,
  const uint16_t *input, unsigned pitch, unsigned width, unsigned height
) {
  pitch >>= 1;
  outpitch >>= 2;

  uint32_t *out0 = output;
  uint32_t *out1 = output + outpitch;

  for(unsigned y = 0; y < height; y++) {
    int prevline = (y == 0 ? 0 : pitch);
    int nextline = (y == height - 1 ? 0 : pitch);

    for(unsigned x = 0; x < width; x++) {
      uint16_t A = *(input - prevline);
      uint16_t B = (x >   0) ? *(input - 1) : *input;
      uint16_t C = *input;
      uint16_t D = (x < width - 1) ? *(input + 1) : *input;
      uint16_t E = *(input++ + nextline);
      uint32_t c = colortable[C];

      if(A != E && B != D) {
        *out0++ = (A == B ? colortable[A] : c);
        if(height <= 240)
          *out1++ = (E == B ? colortable[E] : c);
        if(width > 256) continue;

        *out0++ = (A == D ? colortable[A] : c);
        if(height <= 240)
          *out1++ = (E == D ? colortable[E] : c);
      } else {
        *out0++ = c;
        if(height <= 240) *out1++ = c;
        if(width > 256) continue;

        *out0++ = c;
        if(height <= 240) *out1++ = c;
      }
    }

    input += pitch - width;
    if(height <= 240) {
      out0 += outpitch + outpitch - 512;
      out1 += outpitch + outpitch - 512;
    } else {
      out0 += outpitch - 512;
    }
  }
}
