common := ../common
nall   := $(common)/nall
include $(nall)/Makefile
snes := snes
ifeq ($(ui),)
  ui := ui-qt
endif

ifeq ($(profile),)
  profile := compatibility
  out     := bsnes
else
  out     := bsnes-$(profile)
endif

objdir := obj/$(profile)

# compiler
c       := $(compiler) -xc -std=gnu99
cpp     := $(compiler) -std=gnu++0x
flags   := -I. -I$(common) -I$(snes)
link    :=
objects :=

# build version
ifeq ($(version),)
  version := 05
else
  flags += -DBSNES_VERSION=\"$(version)\"
endif

ifeq ($(DEBUG), 1)
  flags += -O0 -g
else
  flags += -O3 -fomit-frame-pointer
  ifeq ($(platform),osx)
    # -s is deprecated as of Apple LLVM 7.0, and using it breaks linking with some versions
  else
    link += -s
  endif
endif

# comment this line to enable asserts
flags += -DNDEBUG

# profile-guided instrumentation
# flags += -fprofile-generate
# link += -lgcov

# silence warnings
flags += -Wno-switch -Wno-absolute-value -Wno-parentheses

# platform
ifeq ($(platform),x)
  link += -ldl -lX11 -lXext
else ifeq ($(platform),osx)
  osxbundle := ../bsnes+.app
  flags += -march=native -mmacosx-version-min=10.10
  link += -F/usr/local/lib -mmacosx-version-min=10.10
else ifeq ($(platform),$(filter $(platform),win msys))
  link += -mwindows
# link += -mconsole
  link += -mthreads -luuid -lkernel32 -luser32 -lgdi32 -lcomctl32 -lcomdlg32 -lshell32 -lole32
  link += -Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc
else
  unknown_platform: help;
endif

# implicit rules
compile = \
  $(strip \
    $(if $(filter %.c,$<), \
      $(c) $(flags) $1 -c $< -o $@, \
      $(if $(filter %.cpp,$<), \
        $(cpp) $(flags) $1 -c $< -o $@ \
      ) \
    ) \
  )

%.o: $<; $(call compile)

all: build plugins;

include $(snes)/Makefile
include $(ui)/Makefile

objects := $(patsubst %,$(objdir)/%.o,$(objects))

# targets
build: ui_build $(objects)
ifeq ($(platform),osx)
	test -d $(osxbundle) || mkdir -p $(osxbundle)/Contents/MacOS
	$(strip $(cpp) -o $(osxbundle)/Contents/MacOS/$(out) $(objects) $(link))
	mkdir -p $(osxbundle)/Contents/Resources
	cp -f data/bsnes.icns $(osxbundle)/Contents/Resources/AppIcon.icns
	sed 's/@VERSION/v$(version)/g' data/Info.plist > $(osxbundle)/Contents/Info.plist
else
#	$(foreach qtdll,$(qtdlls),xcopy $(winqtpath)\bin\$(qtdll) .\out\ /D /Y &)
#	$(foreach qtplatformdll,$(qtplatformdlls),xcopy $(winqtpath)\$(qtplatformdll) .\out\platforms\ /D /Y &)
	$(strip $(cpp) -o out/$(out) $(objects) $(link))
endif

install: build plugins
ifeq ($(platform),x)
	install -D -m 755 out/$(out) $(DESTDIR)$(prefix)/bin/$(out)
	install -D -m 644 data/bsnes.png $(DESTDIR)$(prefix)/share/pixmaps/bsnes.png
	install -D -m 644 data/bsnes.desktop $(DESTDIR)$(prefix)/share/applications/bsnes.desktop
	test -d ~/.bsnes || mkdir ~/.bsnes
else ifeq ($(platform),osx)
	mv -f $(osxbundle) /Applications/bsnes+.app
	test -d ~/.bsnes || mkdir ~/.bsnes
else
	@echo Install not available for current platform
endif

uninstall:
ifeq ($(platform),x)
	rm $(DESTDIR)$(prefix)/bin/$(out)
	rm $(DESTDIR)$(prefix)/share/pixmaps/bsnes.png
	rm $(DESTDIR)$(prefix)/share/applications/bsnes.desktop
else ifeq ($(platform),osx)
	rm -rf /Applications/bsnes+.app
else
	@echo Install not available for current platform
endif

plugins: build
	@$(MAKE) -C ../snesreader
	@$(MAKE) -C ../snesfilter
	@$(MAKE) -C ../snesmusic
	@$(MAKE) -C ../supergameboy
ifeq ($(platform),osx)
	mkdir -p $(osxbundle)/Contents/Frameworks
	cp -f ../snesreader/libsnesreader.dylib $(osxbundle)/Contents/Frameworks/libsnesreader.dylib
	cp -f ../snesfilter/libsnesfilter.dylib $(osxbundle)/Contents/Frameworks/libsnesfilter.dylib
	cp -f ../snesmusic/libsnesmusic.dylib $(osxbundle)/Contents/Frameworks/libsnesmusic.dylib
	cp -f ../supergameboy/libsupergameboy.dylib $(osxbundle)/Contents/Frameworks/libsupergameboy.dylib
endif

distribution: clean build plugins
ifeq ($(platform),osx)
	@rm -f ../bsnes_v$(version)_osx.zip
	@zip -9 -r ../bsnes_v$(version)_osx.zip $(osxbundle)
	@rm -rf $(osxbundle)
else
	@echo Distribution target not available for current platform
endif

clean: ui_clean plugins_clean
	-@$(call delete,$(objdir)/*.o)
	-@$(call delete,$(objdir)/*.a)
	-@$(call delete,$(objdir)/*.so)
	-@$(call delete,$(objdir)/*.dylib)
	-@$(call delete,$(objdir)/*.dll)
	-@$(call delete,*.res)
	-@$(call delete,*.pgd)
	-@$(call delete,*.pgc)
	-@$(call delete,*.ilk)
	-@$(call delete,*.pdb)
	-@$(call delete,*.manifest)
ifeq ($(platform),osx)
	@rm -rf $(osxbundle)
endif

plugins_clean:
	@$(MAKE) clean -C ../snesreader
	@$(MAKE) clean -C ../snesfilter
	@$(MAKE) clean -C ../snesmusic
	@$(MAKE) clean -C ../supergameboy

archive-all:
	tar -cjf bsnes.tar.bz2 data launcher libco obj out ruby snes ui-qt Makefile cc.bat clean.bat sync.sh uname.bat

help:;
