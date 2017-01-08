class Music : public library {
public:
  string extensionList;

  function<const char* ()> supported;
  function<void ()> unload;
  function<bool ()> loaded;
  function<bool (string&, uint8_t*&, uint16_t&, uint8_t(&)[4], uint8_t&)> load_spc;
  function<bool (string&, uint8_t*&, unsigned&)> load_snsf;
  function<void (uint16_t*, unsigned, unsigned, unsigned)> render;

  const char* default_supported();
  void default_unload();
  bool default_loaded();
  bool default_load_spc(string&, uint8_t*&, uint16_t&, uint8_t(&)[4], uint8_t&);
  bool default_load_snsf(string&, uint8_t*&, unsigned&);
  void default_render(uint16_t*, unsigned, unsigned, unsigned);

  Music();
};

extern Music music;
