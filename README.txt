bsnes-plus (or bsnes+) is a fork of bsnes (based on bsnes-classic v073u1) intended to
introduce some new features and improvements, mostly aimed at debugging.

Building on Windows:

- Get MinGW
- Get the latest DirectX SDK; copy its header files into MinGW's include dir,
  overwriting any existing files
  (Additional DirectX headers and libraries no longer included with the SDK
   are included in bsnes/directx/)
- Get the latest version of glext.h from https://www.opengl.org/registry/#headers
  and put it in MinGW's include/GL, overwriting the old one
- Run mingw32-make

Building on OS X / Unix

- Run make (and hope it works)

Building on all platforms requires Qt 4 installed.

The snesfilter, snesreader, and supergameboy plugins can all be built by running make
(or mingw32-make) after you've configured your environment to build bsnes itself.
After building, just copy the .dll, .so, or .dylib files into the same directory as
bsnes itself.

This fork of bsnes doesn't include the alternate UI based on byuu's `phoenix` library.
The purpose of this fork is primarily to add additional UI functionality and I have
no intention of implementing every new feature twice using completely different libraries
just to keep both versions of the UI at parity.

bsnes v073 and its derivatives are licensed under the GPL v2; see Help > License...
for more information.