#include "symbol_file_interface.hpp"

class FmaSymbolFile : public SymbolFileInterface {
public:
  FmaSymbolFile() {}
  virtual ~FmaSymbolFile() {}

  const char *getName() const;
  const char *getDescription() const;

  int32_t scoreReadString(const lstring &rows) const;
  bool read(const lstring &rows, SymbolMap *map) const;

  bool write(nall::file &file, SymbolMap *map) const;

  uint32_t getFeatures() const;

protected:
  string filteredRow(const string &input) const;
  void readSymbol(SymbolMap *map, const lstring &args) const;
  void readComment(SymbolMap *map, const lstring &args) const;

  string writeAddress(uint32_t address) const;

  uint32_t parseAddress(const string &arg) const;
  void parseArgs(const string &row, lstring &args) const;
};
