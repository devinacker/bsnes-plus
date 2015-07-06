#include "snesfilter.hpp"

#if defined(_WIN32)
  #define bsnesexport __declspec(dllexport)
#else
  #define bsnesexport
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QT_CORE_LIB
#include <QtGui>

#include <nall/config.hpp>
#include <nall/platform.hpp>
#include <nall/string.hpp>
using namespace nall;

const uint32_t *colortable;

#include "direct/direct.cpp"
#include "pixellate2x/pixellate2x.cpp"
#include "scale2x/scale2x.cpp"
#include "2xsai/2xsai.cpp"
#include "lq2x/lq2x.cpp"
#include "hq2x/hq2x.cpp"
#include "ntsc/ntsc.cpp"
#include "xbrz/xbrzfilter.cpp"

bsnesexport const char* snesfilter_supported() {
  return "Pixellate2x;Scale2x;2xSaI;Super 2xSaI;Super Eagle;LQ2x;HQ2x;xBRZ;NTSC";
}

bsnesexport void snesfilter_configuration(configuration &config) {
  // A reference should never be null
  filter_xbrz.bind(config);
  filter_ntsc.bind(config);
}

bsnesexport void snesfilter_colortable(const uint32_t *colortable_) {
  colortable = colortable_;
}

bsnesexport void snesfilter_size(unsigned filter, unsigned &outwidth, unsigned &outheight, unsigned width, unsigned height) {
  switch(filter) {
    default: return filter_direct.size(outwidth, outheight, width, height);
    case 1:  return filter_pixellate2x.size(outwidth, outheight, width, height);
    case 2:  return filter_scale2x.size(outwidth, outheight, width, height);
    case 3:  return filter_2xsai.size(outwidth, outheight, width, height);
    case 4:  return filter_super2xsai.size(outwidth, outheight, width, height);
    case 5:  return filter_supereagle.size(outwidth, outheight, width, height);
    case 6:  return filter_lq2x.size(outwidth, outheight, width, height);
    case 7:  return filter_hq2x.size(outwidth, outheight, width, height);
    case 8:  return filter_xbrz.size(outwidth, outheight, width, height);
    case 9:  return filter_ntsc.size(outwidth, outheight, width, height);
  }
}

bsnesexport void snesfilter_render(
  unsigned filter, uint32_t *output, unsigned outpitch,
  const uint16_t *input, unsigned pitch, unsigned width, unsigned height
) {
  switch(filter) {
    default: return filter_direct.render(output, outpitch, input, pitch, width, height);
    case 1:  return filter_pixellate2x.render(output, outpitch, input, pitch, width, height);
    case 2:  return filter_scale2x.render(output, outpitch, input, pitch, width, height);
    case 3:  return filter_2xsai.render(output, outpitch, input, pitch, width, height);
    case 4:  return filter_super2xsai.render(output, outpitch, input, pitch, width, height);
    case 5:  return filter_supereagle.render(output, outpitch, input, pitch, width, height);
    case 6:  return filter_lq2x.render(output, outpitch, input, pitch, width, height);
    case 7:  return filter_hq2x.render(output, outpitch, input, pitch, width, height);
    case 8:  return filter_xbrz.render(output, outpitch, input, pitch, width, height);
    case 9:  return filter_ntsc.render(output, outpitch, input, pitch, width, height);
  }
}

bsnesexport QWidget* snesfilter_settings(unsigned filter) {
  switch(filter) {
    default: return 0;
    case 8:  return filter_xbrz.settings();
    case 9:  return filter_ntsc.settings();
  }
}
