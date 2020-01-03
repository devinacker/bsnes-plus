common := ../common
nall   := $(common)/nall
include $(nall)/Makefile

c     := $(compiler) -x c -std=gnu99
cpp   := $(compiler) -std=gnu++0x
flags := -O3 -fomit-frame-pointer -I. -I$(common) -Icommon -Ilibgambatte/include -Ilibgambatte/src
link  :=

ifeq ($(platform),x)
  flags := -fPIC $(flags)
  link += -s
else ifeq ($(platform),osx)
  flags += -fPIC -march=native -mmacosx-version-min=10.10
  link += -F/usr/local/lib -mmacosx-version-min=10.10
endif

objects := supergameboy
objects += bitmap_font cpu gambatte initstate interrupter interruptrequester
objects += loadres memory sound state_osd_elements statesaver tima video
objects += cartridge memptrs pakinfo rtc
objects += channel1 channel2 channel3 channel4 duty_unit envelope_unit length_counter
objects += ly_counter lyc_irq next_m0_time ppu sprite_mapper
objects += file

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

####################
### supergameboy ###
####################

obj/supergameboy.o: supergameboy.cpp *.cpp *.hpp $(call rwildcard,interface/)

###################
### libgambatte ###
###################

obj/bitmap_font.o: libgambatte/src/bitmap_font.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/cpu.o: libgambatte/src/cpu.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/gambatte.o: libgambatte/src/gambatte.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/initstate.o: libgambatte/src/initstate.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/interrupter.o: libgambatte/src/interrupter.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/interruptrequester.o: libgambatte/src/interruptrequester.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/loadres.o: libgambatte/src/loadres.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/memory.o: libgambatte/src/memory.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/sound.o: libgambatte/src/sound.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/state_osd_elements.o: libgambatte/src/state_osd_elements.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/statesaver.o: libgambatte/src/statesaver.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/tima.o: libgambatte/src/tima.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/video.o: libgambatte/src/video.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)

obj/cartridge.o: libgambatte/src/mem/cartridge.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/memptrs.o: libgambatte/src/mem/memptrs.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/pakinfo.o: libgambatte/src/mem/pakinfo.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/rtc.o: libgambatte/src/mem/rtc.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)

obj/channel1.o: libgambatte/src/sound/channel1.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/channel2.o: libgambatte/src/sound/channel2.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/channel3.o: libgambatte/src/sound/channel3.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/channel4.o: libgambatte/src/sound/channel4.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/duty_unit.o: libgambatte/src/sound/duty_unit.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/envelope_unit.o: libgambatte/src/sound/envelope_unit.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/length_counter.o: libgambatte/src/sound/length_counter.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)

obj/ly_counter.o: libgambatte/src/video/ly_counter.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/lyc_irq.o: libgambatte/src/video/lyc_irq.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/next_m0_time.o: libgambatte/src/video/next_m0_time.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/ppu.o: libgambatte/src/video/ppu.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)
obj/sprite_mapper.o: libgambatte/src/video/sprite_mapper.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)

obj/file.o: libgambatte/src/file/file.cpp $(call rwildcard,common/) $(call rwildcard,libgambatte/)

###############
### targets ###
###############

build: $(objects)
ifeq ($(platform),$(filter $(platform),win msys))
	$(cpp) $(link) -o supergameboy.dll -shared -Wl,--out-implib,libsupergameboy.a $(objects) $(qtlib)
else ifeq ($(platform),osx)
	ar rcs libsupergameboy.a $(objects)
	$(cpp) $(link) -o libsupergameboy.dylib -shared -dynamiclib $(objects) $(qtlib)
else
	ar rcs libsupergameboy.a $(objects)
	$(cpp) $(link) -o libsupergameboy.so -shared -Wl,-soname,libsupergameboy.so.1 $(objects) $(qtlib)
endif

install:
ifeq ($(platform),x)
	install -D -m 755 libsupergameboy.a $(DESTDIR)$(prefix)/lib
	install -D -m 755 libsupergameboy.so $(DESTDIR)$(prefix)/lib
	ldconfig -n $(DESTDIR)$(prefix)/lib
else
	@echo Install not available for current platform
endif

clean:
	-@$(call delete,obj/*.o)
	-@$(call delete,libsupergameboy.a)
	-@$(call delete,supergameboy.dll)
	-@$(call delete,libsupergameboy.dylib)
	-@$(call delete,libsupergameboy.so)
