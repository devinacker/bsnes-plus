#ifndef SYMBOL_FILE_INTERFACE__H
#define SYMBOL_FILE_INTERFACE__H

class SymbolFileInterface {
public:
  SymbolFileInterface() {}
  virtual ~SymbolFileInterface() {}

  virtual const char *getName() const = 0;
  virtual const char *getDescription() const = 0;
  virtual uint32_t getFeatures() const = 0;

  // Readable
  virtual int32_t scoreReadString(const nall::lstring &rows) const {
    return -1;
  };
  virtual bool read(const nall::lstring &rows, SymbolMap *map) const {
    return false;
  };

  // Writable
  virtual bool write(nall::file &file, SymbolMap *map) const {
    return false;
  }

  enum SymbolFileFeature {
    Readable = 0x0001,
    Writable = 0x0002,

    Symbols = 0x0010,
    Comments = 0x0020,
    DebugInterface = 0x0040,
    Files = 0x0080,
    LineMap = 0x0100
  };

};

#endif
