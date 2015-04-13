# bsnes-plus

bsnes-plus (or bsnes+) is a fork of bsnes (based on bsnes-classic) intended to
introduce some new features and improvements, mostly aimed at debugging.

## What's new

- "Step over" and "step out" buttons in debugger
- Improved handling of address mirroring for breakpoints (extends to the entire address space, not just RAM)
- Real-time code and data highlighting in memory editor, with fast searching for known code/data locations and unexplored regions
- Cartridge ROM and RAM views in memory editor for mapper-agnostic analysis
- SA-1 disassembly and debugging
- SA-1 bus and BW-RAM viewing and (partial) usage logging
- Super FX disassembly and debugging
- Super FX bus viewing and usage logging

## Coming soon

- Improved debugger UI with register editing
- More coprocesor status viewing in properties window
- On-the-fly ROM saving and reloading from the memory editor for quick hacking and testing
- More keyboard shortcuts for menus, etc.
- Similar addressing improvements for cheats

## Building on Windows

- Get MinGW
- Get the latest DirectX SDK; copy its header files into MinGW's include dir, overwriting any existing files  
  (Additional DirectX headers and libraries no longer included with the SDK are included in `bsnes/directx/`)
- Get the latest version of `glext.h` from [opengl.org](https://www.opengl.org/registry/#headers) and put it in MinGW's `include/GL/`, overwriting the old one
- Run `mingw32-make`

## Building on OS X / Unix

- Run `make` (and hope it works)

Building on all platforms requires Qt 4 installed.

The snesfilter, snesreader, and supergameboy plugins can all be built by running make (or mingw32-make) after you've configured your environment to build bsnes itself.
After building, just copy the .dll, .so, or .dylib files into the same directory as bsnes itself.

This fork of bsnes doesn't include the alternate UI based on byuu's `phoenix` library. The purpose of this fork is primarily to add additional UI functionality and I have no intention of implementing every new feature twice using completely different libraries just to keep both versions of the UI at parity.

bsnes v073 and its derivatives are licensed under the GPL v2; see *Help > License...* for more information.
