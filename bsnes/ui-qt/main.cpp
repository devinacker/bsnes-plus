#include "ui-base.hpp"
#include "resource.rcc"

#if defined(PLATFORM_X)
  #include "platform/platform_x.cpp"
  const char Style::Monospace[64] = "monospace";
#elif defined(PLATFORM_OSX)
  #include "platform/platform_osx.cpp"
  const char Style::Monospace[64] = "Courier New";
#elif defined(PLATFORM_WIN)
  #include "platform/platform_win.cpp"
  const char Style::Monospace[64] = "Lucida Console";
#else
  #error "unsupported platform"
#endif

#include "config.cpp"
#include "interface.cpp"

const char defaultStylesheet[] =
  "#backdrop {"
  "  background: #000000;"
  "}\n";

#include "check-action.moc"
#include "check-delegate.moc"
#include "combo-delegate.moc"
#include "file-dialog.moc"
#include "radio-action.moc"
#include "window.moc"

#include "application/application.cpp"
#include "link/filter.cpp"
#include "link/music.cpp"
#include "link/reader.cpp"
#include "utility/utility.cpp"

int main(int argc, char **argv) {
  return application.main(argc, argv);
}

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
