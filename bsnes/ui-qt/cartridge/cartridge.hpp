class Cartridge {
public:
  string name;        //printable name
  string fileName;    //ideal file name for saving data to disk
  string baseName;    //physical cartridge file name
  string slotAName;   //Sufami Turbo slot A file name or BS-X slot file name
  string slotBName;   //Sufami Turbo slot B file name
  string patchApplied;  //filename of the patch if one was applied to image

  string baseXml;
  string slotAXml;
  string slotBXml;

  struct Information {
    string name;
    string region;
    unsigned romSize;
    unsigned ramSize;
  };

  bool information(const char*, Information&);
  bool saveStatesSupported();

  bool loadNormal(const char*);
  bool loadBsxSlotted(const char*, const char*);
  bool loadBsx(const char*, const char*);
  bool loadSufamiTurbo(const char*, const char *, const char*);
  bool loadSuperGameBoy(const char*, const char*);
  bool loadSpc(const char*);
  bool loadSnsf(const char*);
  void saveMemory();
  void saveMemoryPack();
  void unload();

  void loadCheats();
  void saveCheats();

private:
  bool loadCartridge(string&, string&, SNES::MappedRAM&);
  bool loadMemory(const char*, const char*, SNES::MappedRAM&);
  bool saveMemory(const char*, const char*, SNES::MappedRAM&);
  bool loadEmptyMemoryPack(string&, SNES::MappedRAM&);
  bool applyBPS(string&, uint8_t *&data, unsigned &size);
  bool applyUPS(string&, uint8_t *&data, unsigned &size);
  bool applyIPS(string&, uint8_t *&data, unsigned &size);
  string decodeJISX0201(const char*);
};

extern Cartridge cartridge;
