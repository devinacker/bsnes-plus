/*
	snesmusic: SPC and SNSF music file loader
	
	Author: Revenant
*/

#include "snesmusic.hpp"

#if defined(_WIN32)
  #define bsnesexport __declspec(dllexport)
#else
  #define bsnesexport
#endif

#define MAX_LIB_DEPTH 10

#include <time.h>
#include <nall/file.hpp>
#include <nall/string.hpp>
#include <zlib/zlib.h>
#include <bitmap_font.h>
#include <map>
using namespace nall;

static struct {
	string artist, title, game;
	time_t length;
	bool loaded = false;
	
} info;

static const char *spc_header = "SNES-SPC700 Sound File Data v0.30\x1a\x1a";

static void read_tag_id666(file &spc);
static bool load_snsf_internal(nall::string &filename, uint8_t *&data, unsigned &size, int depth = 0);
static const char* string_convert(const char *in, unsigned maxwidth);

bsnesexport const char* snesmusic_supported() {
	return "*.spc *.snsf *.minisnsf";
}

bsnesexport void snesmusic_unload() {
	info.loaded = false;
	info.length  = 0;
	info.artist = info.title = info.game = string();
}

bsnesexport bool snesmusic_loaded() {
	return info.loaded;
}

bsnesexport bool snesmusic_load_spc(string &filename, uint8_t *&dump,
                                    uint16_t &pc, uint8_t (&regs)[4], uint8_t &p) {

	file spc;
	char header[sizeof(spc_header)];
		
	snesmusic_unload();
	
	if (!spc.open(filename, file::mode::read))
		return false;
	
	if (spc.size() < 0x10200)
		return false;
	
	// validate file header
	spc.read((uint8_t*)header, sizeof(header));
	if (memcmp(header, spc_header, sizeof(header)))
		return false;

	// read SPC700 registers
	spc.seek(0x25);
	pc      = spc.readl(2);
	regs[0] = spc.read();
	regs[1] = spc.read();
	regs[2] = spc.read();
	p       = spc.read();
	regs[3] = spc.read();
	
	// read APU RAM and DSP registers
	spc.seek(0x100);
	spc.read(dump, 0x10000 + 128);
	
	info.loaded = true;
	
	read_tag_id666(spc);
	return true;
}

void read_tag_id666(file &spc) {
	char id_tag[32+1];
	id_tag[32] = '\0';

	spc.seek(0x2e);
	
	// song title and game title are the same in both tag formats
	spc.read((uint8_t*)id_tag, 32);
	info.title = id_tag;
	spc.read((uint8_t*)id_tag, 32);
	info.game = id_tag;
	
	// determine text or binary tag format
	// (though we only need length and song artist)
	bool binary = false;
	// check length fields
	spc.seek(0xa9);
	for (int i = 0; i < 7; i++) {
		uint8_t data = spc.read();
		if ((data > 0 && data < 0x20) || data > 0x7e) {
			binary = true;
			break;
		}
	}
	// check first byte of artist field
	if (!binary) {
		if (spc.read() >= 'A') {
			binary = true;
		}
	}
	
	// read artist and length fields once we know the format
	if (binary) {
		// length
		spc.seek(0xa9);
		info.length = spc.readl(3);
		
		// artist
		spc.seek(0xb0);		
		spc.read((uint8_t*)id_tag, 32);
		info.artist = id_tag;
		
	} else {
		// length
		spc.seek(0xa9);
		spc.read((uint8_t*)id_tag, 3);
		id_tag[3] = '\0';
		info.length = integer(id_tag);
		
		// artist
		spc.seek(0xb1);		
		spc.read((uint8_t*)id_tag, 32);
		info.artist = id_tag;
		
	}
}

bsnesexport bool snesmusic_load_snsf(nall::string &filename, uint8_t *&data, unsigned &size) {
	snesmusic_unload();

	data = 0;
	size = 0;

	return info.loaded = load_snsf_internal(filename, data, size);
}

bool load_snsf_internal(nall::string &filename, uint8_t *&data, unsigned &size, int depth) {
	if (depth >= MAX_LIB_DEPTH)
		return false;
	
	file snsf;
	if (!snsf.open(filename, file::mode::read))
		return false;
	
	// validate file header
	if (snsf.read() != 'P' || 
	    snsf.read() != 'S' ||
	    snsf.read() != 'F' || 
	    snsf.read() != '\x23')
		// not a SNSF file
		return false;
	
	uint32_t file_size     = snsf.size();
	uint32_t reserved_size = snsf.readl(4);
	uint32_t program_size  = snsf.readl(4);
	uint32_t data_size     = 16 + reserved_size + program_size;
	uint32_t tag_size      = file_size - data_size;
	uint32_t checksum      = snsf.readl(4);
	
	if (file_size < data_size)
		return false;
	
	std::map<string, string> tags;
	int libn = 2;
	
	// load compressed program data
	uint8_t *data_in = new uint8_t[program_size];
	snsf.seek(16 + reserved_size);
	snsf.read(data_in, program_size);
	
	// check CRC32
	if (checksum != crc32(crc32(0, 0, 0), data_in, program_size))
		goto error_input;
	
	// parse tags and load snsflib
	snsf.seek(data_size);
	if (snsf.read() == '[' && 
	    snsf.read() == 'T' &&
	    snsf.read() == 'A' &&
	    snsf.read() == 'G' &&
	    snsf.read() == ']') {
		
		tag_size -= 5;
		
		uint8_t *tag_data = new uint8_t[tag_size+1];
		tag_data[tag_size] = '\0';
		snsf.read(tag_data, tag_size);
		
		lstring _tags;
		_tags.split("\n", (char*)tag_data);
		delete[] tag_data;
		
		for(unsigned i = 0; i < _tags.size(); i++) {
			lstring pair;
			pair.split<1>("=", _tags[i]);
			
			if (pair.size() < 2)
				continue;
			
			tags[pair[0].lower()] = pair[1];
		}
	}
	
	// load main snsflib
	if (tags.count("_lib")) {
		string libname = dir(filename) << tags["_lib"];
		
		if (!load_snsf_internal(libname, data, size, depth+1))
			goto error_input;
	}
	
	// load program data
	uint32_t offset, rom_size;
	uint8_t temp[4];
	
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	if (inflateInit(&stream) != Z_OK) 
		goto error_input;
	
	stream.avail_in = program_size;
	stream.next_in  = data_in;
	
	// read program offset
	stream.avail_out = sizeof(temp);
	stream.next_out  = temp;
	
	if (inflate(&stream, Z_NO_FLUSH) != Z_OK)
		goto error_stream;
	offset = temp[0] | (temp[1] << 8) | (temp[2] << 16) | (temp[3] << 24);
	
	// read ROM size
	stream.avail_out = sizeof(temp);
	stream.next_out  = temp;
	
	if (inflate(&stream, Z_NO_FLUSH) != Z_OK)
		goto error_stream;
	rom_size = temp[0] | (temp[1] << 8) | (temp[2] << 16) | (temp[3] << 24);
	
	// allocate (or reallocate) ROM
	if ((rom_size + offset) > size) {
		uint8_t *data_out = new uint8_t[rom_size + offset];
		
		if (size > 0) {
			memcpy(data_out, data, size);
			delete[] data;
		}
		
		size = rom_size + offset;
		data = data_out;
	}
	
	// decompress ROM data
	stream.avail_out = rom_size;
	stream.next_out  = data + offset;
	
	inflate(&stream, Z_NO_FLUSH);
	inflateEnd(&stream);
	
	// don't need compressed input data anymore
	delete[] data_in;
	data_in = 0;
	
	// TODO: handle reserved data
	
	// metadata tags
	if (tags.count("title"))
		info.title = tags["title"];
	if (tags.count("artist"))
		info.artist = tags["artist"];
	if (tags.count("game"))
		info.game = tags["game"];
	
	// load additional snsflibs
	while (1) {
		string tag = string("_lib") << integer(libn++);
		
		if (tags.count(tag)) {
			string libname = dir(filename) << tags[tag];
			
			if (!load_snsf_internal(libname, data, size, depth+1)) 
				goto error_output;
		} else {
			break;
		}
	}
	
	return true;

error_output:
	delete[] data;
	data = 0;
	size = 0;
error_stream:
	inflateEnd(&stream);
error_input:
	delete[] data_in;
	
	return false;
}

/*
	Terrible, lazy text drawing with the BitmapFont code borrowed/stolen from gambatte.
	I'd like to clean up/expand this into a proper interface for drawing text (and more)
	which would live in the actual application instead of the music plugin, so that
	it could be used for other stuff in the future. But that's not important yet...
	
	(this also assumes no mid-screen resolution changes, which is fine for now assuming
	 we don't have any badly-made SNSF files that mess around with PPU registers)
*/

template <uint16_t color = 0x7fff>
static void point_shadow(uint16_t *dest, unsigned pitch) {
	const uint16_t shadow = (color >> 1) & 0x3DEF;
	*(dest) = color;
	*(dest + 1) = *(dest + pitch) = *(dest + pitch + 1) = shadow;
}

bsnesexport void snesmusic_render(uint16_t *data, unsigned pitch, unsigned width, unsigned height) {
	using namespace BitmapFont;

#define PAD 8
#define PXL(x,y) (data + (x) + (y)*pitch)

	width  -= PAD*2;
	height -= PAD*2;
	// height is currently unused
	// TODO: clip text to height if needed, but we're not drawing that much right now
	
	// show title
	const char titleStr[] = {T, i, t, l, e, 0};
	print(PXL(PAD+0, PAD+0), pitch, point_shadow<0x001f>, titleStr);
	print(PXL(PAD+40, PAD+0), pitch, point_shadow<0x7fff>, string_convert(info.title, width-40));
	// show artist
	const char artistStr[] = {A, r, t, i, s, t, 0};
	print(PXL(PAD+0, PAD+HEIGHT), pitch, point_shadow<0x001f>, artistStr);
	print(PXL(PAD+40, PAD+HEIGHT), pitch, point_shadow<0x7fff>, string_convert(info.artist, width-40));
	// show game name
	const char gameStr[] = {G, a, m, e, 0};
	print(PXL(PAD+0, PAD+HEIGHT*2), pitch, point_shadow<0x001f>, gameStr);
	print(PXL(PAD+40, PAD+HEIGHT*2), pitch, point_shadow<0x7fff>, string_convert(info.game, width-40));
	
#undef PAD
#undef PXL
}

const char* string_convert(const char *in, unsigned maxwidth) {
	static char out[256];
	
	int i = 0;
	for (; in[i] && (i < 255); i++) {
		if (in[i] >= '0' && in[i] <= '9')
			out[i] = BitmapFont::N0 + in[i] - '0';
		else if (in[i] >= 'A' && in[i] <= 'Z')
			out[i] = BitmapFont::A + in[i] - 'A';
		else if (in[i] >= 'a' && in[i] <= 'z')
			out[i] = BitmapFont::a + in[i] - 'a';
		else
			out[i] = BitmapFont::SPC;
	}
	do {
		out[i--] = '\0';
	} while ((BitmapFont::getWidth(out) >= maxwidth) && (i >= 0));
	
	return out;
}
