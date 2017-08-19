#define UNICODE
#define QT_NO_DEBUG
#define QT_THREAD_SUPPORT

#include <QApplication>
#include <QtWidgets>
//Q_IMPORT_PLUGIN(QJpegPlugin)
//Q_IMPORT_PLUGIN(QMngPlugin)

#include <snes.hpp>

#include <nall/base64.hpp>
#include <nall/config.hpp>
#include <nall/input.hpp>
#include <nall/ups.hpp>
#include <nall/snes/cartridge.hpp>
#include "template/concept.hpp"
#include "template/check-action.moc.hpp"
#include "template/file-dialog.moc.hpp"
#include "template/radio-action.moc.hpp"
#include "template/window.moc.hpp"
using namespace nall;

#include <ruby/ruby.hpp>
using namespace ruby;

#include "config.hpp"
#include "interface.hpp"

#include "application/application.moc.hpp"

#include "base/about.moc.hpp"
#include "base/filebrowser.moc.hpp"
#include "base/htmlviewer.moc.hpp"
#include "base/loader.moc.hpp"
#include "base/main.moc.hpp"
#include "base/stateselect.moc.hpp"

#include "cartridge/cartridge.hpp"

#if defined(DEBUGGER)
  #include "debugger/debugger.moc.hpp"
  #include "debugger/tracer.moc.hpp"
  #include "debugger/registeredit.moc.hpp"

  #include "debugger/tools/disassembler.moc.hpp"
  #include "debugger/tools/breakpoint.moc.hpp"
  #include "debugger/tools/memory.moc.hpp"
  #include "debugger/tools/properties.moc.hpp"

  #include "debugger/ppu/cgram-widget.moc.hpp"
  #include "debugger/ppu/image-grid-widget.moc.hpp"
  #include "debugger/ppu/tilemap-renderer.hpp"

  #include "debugger/ppu/vram-viewer.moc.hpp"
  #include "debugger/ppu/tilemap-viewer.moc.hpp"
  #include "debugger/ppu/oam-viewer.moc.hpp"
  #include "debugger/ppu/cgram-viewer.moc.hpp"

  #include "debugger/misc/debugger-options.moc.hpp"
#endif

#include "input/input.hpp"

#include "link/filter.hpp"
#include "link/music.hpp"
#include "link/reader.hpp"

#include "movie/movie.hpp"

#include "settings/settings.moc.hpp"
#if defined(LAUNCHER)
  #include "settings/profile.moc.hpp"
#endif
#include "settings/video.moc.hpp"
#include "settings/audio.moc.hpp"
#include "settings/input.moc.hpp"
#include "settings/paths.moc.hpp"
#include "settings/advanced.moc.hpp"
#include "settings/bsx.moc.hpp"

#include "state/state.hpp"

#include "tools/tools.moc.hpp"
#include "tools/cheateditor.moc.hpp"
#include "tools/cheatfinder.moc.hpp"
#include "tools/statemanager.moc.hpp"
#include "tools/effecttoggle.moc.hpp"
#include "tools/soundviewer.moc.hpp"

#include "utility/utility.hpp"

struct Style {
  static const char Monospace[64];
  static int MonospaceSize;

  enum {
    WindowMargin     = 5,
    WidgetSpacing    = 5,
    SeparatorSpacing = 5,
  };
};

extern string filepath(const char *filename, const char *filepath);
