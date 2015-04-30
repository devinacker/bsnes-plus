include nall/Makefile
snes := snes
ifeq ($(profile),)
  profile := compatibility
endif
ifeq ($(ui),)
  ui := ui-qt
endif

# compiler
c       := $(compiler) -x c -std=gnu99
cpp     := $(compiler) -std=gnu++0x
flags   := -I. -I$(snes) -fomit-frame-pointer
link    :=
objects :=

# profile-guided instrumentation
# flags += -fprofile-generate
# link += -lgcov

# profile-guided optimization
# flags += -fprofile-use

# silence warnings
flags += -Wno-switch -Wno-absolute-value -Wno-parentheses

# build for debugging if $(DEBUG) == 1, otherwise optimize
# (investigate: why does the build crash when using -O0 just a few emulated cycles?)
ifeq ($(DEBUG), 1)
  flags += -O1 -g
else
  flags += -O3
endif

# platform
ifeq ($(platform),x)
  link += -s -ldl -lX11 -lXext
else ifeq ($(platform),osx)
	osxbundle := ../bsnes+.app
else ifeq ($(platform),win)
  link += -mwindows
# link += -mconsole
  link += -mthreads -s -luuid -lkernel32 -luser32 -lgdi32 -lcomctl32 -lcomdlg32 -lshell32 -lole32
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

all: build;

include $(snes)/Makefile
include $(ui)/Makefile

objects := $(patsubst %,obj/%.o,$(objects))

# targets
build: ui_build $(objects)
ifeq ($(platform),osx)
	test -d $(osxbundle) || mkdir -p $(osxbundle)/Contents/MacOS
	$(strip $(cpp) -o $(osxbundle)/Contents/MacOS/bsnes $(objects) $(link))
	mkdir -p $(osxbundle)/Contents/Resources
	cp -f data/bsnes.icns $(osxbundle)/Contents/Resources/AppIcon.icns
	cp -f data/Info.plist $(osxbundle)/Contents/Info.plist
else
	$(strip $(cpp) -o out/bsnes $(objects) $(link))
endif

install:
ifeq ($(platform),x)
	install -D -m 755 out/bsnes $(DESTDIR)$(prefix)/bin/bsnes
	install -D -m 644 data/bsnes.png $(DESTDIR)$(prefix)/share/pixmaps/bsnes.png
	install -D -m 644 data/bsnes.desktop $(DESTDIR)$(prefix)/share/applications/bsnes.desktop
	test -d ~/.bsnes || mkdir ~/.bsnes
	cp data/cheats.xml ~/.bsnes/cheats.xml
	chmod 777 ~/.bsnes ~/.bsnes/cheats.xml
endif

uninstall:
ifeq ($(platform),x)
	rm $(DESTDIR)$(prefix)/bin/bsnes
	rm $(DESTDIR)$(prefix)/share/pixmaps/bsnes.png
	rm $(DESTDIR)$(prefix)/share/applications/bsnes.desktop
endif

clean: ui_clean
	-@$(call delete,obj/*.o)
	-@$(call delete,obj/*.a)
	-@$(call delete,obj/*.so)
	-@$(call delete,obj/*.dylib)
	-@$(call delete,obj/*.dll)
	-@$(call delete,*.res)
	-@$(call delete,*.pgd)
	-@$(call delete,*.pgc)
	-@$(call delete,*.ilk)
	-@$(call delete,*.pdb)
	-@$(call delete,*.manifest)
ifeq ($(platform),osx)
	rm -rf $(osxbundle)
endif

archive-all:
	tar -cjf bsnes.tar.bz2 data launcher libco nall obj out phoenix ruby snes ui-phoenix ui-qt Makefile cc.bat clean.bat sync.sh

help:;
