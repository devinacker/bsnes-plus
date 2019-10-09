common := ../common
nall   := $(common)/nall
zlib   := $(common)/zlib
include $(nall)/Makefile

c     := $(compiler) -x c -std=gnu99
cpp   := $(compiler) -std=gnu++0x
flags := -I. -I$(common)
link  :=

ifeq ($(DEBUG), 1)
  flags += -O0 -g
else
  flags += -O3 -fomit-frame-pointer
endif

# silence warnings
flags += -Wno-switch -Wno-absolute-value -Wno-parentheses

ifeq ($(platform),x)
  flags := -fPIC $(flags)
  link += -s
else ifeq ($(platform),osx)
  flags += -fPIC -march=native -mmacosx-version-min=10.10
  link += -F/usr/local/lib -mmacosx-version-min=10.10
endif

objects := snesmusic
# bitmap_font
objects += bitmap_font
# zlib
objects += adler32 crc32 inffast inflate inftrees zutil

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

objects := $(patsubst %,obj/%.o,$(objects))

#################
### snesmusic ###
#################

obj/snesmusic.o: snesmusic.cpp *.cpp *.hpp

###################
### bitmap_font ###
###################

obj/bitmap_font.o: bitmap_font.cpp

############
### zlib ###
############

obj/adler32.o : $(zlib)/adler32.c $(zlib)/*
obj/crc32.o   : $(zlib)/crc32.c $(zlib)/*
obj/inffast.o : $(zlib)/inffast.c $(zlib)/*
obj/inflate.o : $(zlib)/inflate.c $(zlib)/*
obj/inftrees.o: $(zlib)/inftrees.c $(zlib)/*
obj/zutil.o   : $(zlib)/zutil.c $(zlib)/*

###############
### targets ###
###############

build: $(objects)
ifeq ($(platform),$(filter $(platform),win msys))
	$(cpp) $(link) -o snesmusic.dll -shared -Wl,--out-implib,libsnesmusic.a $(objects) $(qtlib)
else ifeq ($(platform),osx)
	ar rcs libsnesmusic.a $(objects)
	$(cpp) $(link) -o libsnesmusic.dylib -shared -dynamiclib $(objects) $(qtlib)
else
	ar rcs libsnesmusic.a $(objects)
	$(cpp) $(link) -o libsnesmusic.so -shared -Wl,-soname,libsnesmusic.so.1 $(objects) $(qtlib)
endif

install:
ifeq ($(platform),x)
	install -D -m 755 libsnesmusic.a $(DESTDIR)$(prefix)/lib
	install -D -m 755 libsnesmusic.so $(DESTDIR)$(prefix)/lib
	ldconfig -n $(DESTDIR)$(prefix)/lib
else
	@echo Install not available for current platform
endif

clean:
	-@$(call delete,obj/*.o)
	-@$(call delete,libsnesmusic.a)
	-@$(call delete,snesmusic.dll)
	-@$(call delete,libsnesmusic.dylib)
	-@$(call delete,libsnesmusic.so)
