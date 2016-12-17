#ifndef NALL_STRING_PLATFORM_HPP
#define NALL_STRING_PLATFORM_HPP

#if defined(PLATFORM_OSX)
#include "CoreFoundation/CoreFoundation.h"
#endif

namespace nall {

string realpath(const char *name) {
  char path[PATH_MAX];
  if(::realpath(name, path)) {
    string result(path);
    result.transform("\\", "/");
    if(result.endswith("/") == false) result.append("/");
    return result;
  }
  return "";
}

string userpath() {
  char path[PATH_MAX];
  if(::userpath(path)) {
    string result(path);
    result.transform("\\", "/");
    if(result.endswith("/") == false) result.append("/");
    return result;
  }
  return "";
}

string currentpath() {
  char path[PATH_MAX];
  if(::getcwd(path)) {
    string result(path);
    result.transform("\\", "/");
    if(result.endswith("/") == false) result.append("/");
    return result;
  }
  return "";
}


string launchpath() {
#if defined(PLATFORM_OSX)
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef bundleURL = CFBundleCopyBundleURL(mainBundle);
  char path[PATH_MAX];
  if (!CFURLGetFileSystemRepresentation(bundleURL, TRUE, (UInt8 *)path, PATH_MAX))
  {
    return "";
  }
  CFRelease(bundleURL);
  return nall::dir(string() << path);
#else
  return currentpath();
#endif
}

}

#endif
