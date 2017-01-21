Music music;

const char* Music::default_supported() {
	return "";
}

void Music::default_unload() {
}

bool Music::default_loaded() {
	return false;
}

bool Music::default_load_spc(string&, uint8_t*&, uint16_t&, uint8_t(&)[4], uint8_t&) {
	return false;
}

bool Music::default_load_snsf(string&, uint8_t*&, unsigned&) {
	return false;
}

void Music::default_render(uint16_t*, unsigned, unsigned, unsigned) {
}

Music::Music() {
#define _sym(n) \
	n = sym("snesmusic_"#n); \
	if (!n) n = { &Music::default_##n, this }

	open("snesmusic");
	
	_sym(supported);
	_sym(unload);
	_sym(loaded);
	_sym(load_spc);
	_sym(load_snsf);
	_sym(render);
	
#undef _sym

	extensionList = supported();
	if(extensionList.length() > 0) extensionList = string() << " " << extensionList;
}
