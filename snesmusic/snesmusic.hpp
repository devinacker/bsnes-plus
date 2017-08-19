#include <stdint.h>
namespace nall { class string; }

extern "C" {
	const char* snesmusic_supported();
	void snesmusic_unload();
	bool snesmusic_loaded();

	bool snesmusic_load_spc(nall::string &filename, uint8_t *&dump,
	                        uint16_t &pc, uint8_t (&regs)[4], uint8_t &p);
	bool snesmusic_load_snsf(nall::string &filename, uint8_t *&data, unsigned &size);

	void snesmusic_render(uint16_t*, unsigned, unsigned, unsigned);
}
