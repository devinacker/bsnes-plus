#include <nall/detect.hpp>
#include <nall/string.hpp>

#if defined(PLATFORM_OSX)
#include "CoreFoundation/CoreFoundation.h"
#endif

nall::string nall::launchpath() {
#if defined(PLATFORM_OSX)
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef bundleURL = CFBundleCopyBundleURL(mainBundle);
  char path[PATH_MAX];
  if (!CFURLGetFileSystemRepresentation(bundleURL, TRUE, (UInt8 *)path, PATH_MAX))
  {
    return "";
  }
  CFRelease(bundleURL);
  return nall::dir(nall::string() << path);
#else
  return nall::currentpath();
#endif
}
