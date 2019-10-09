#include "symbol_file_interface.hpp"

class SymbolMap;

class ViceLabelFile : public SymbolFileInterface {
public:
  ViceLabelFile() {}
  virtual ~ViceLabelFile() {}

  const char *getName() const;
  const char *getDescription() const;

  int32_t scoreReadString(const nall::lstring &rows) const;
  bool read(const nall::lstring &rows, SymbolMap *map) const;

  uint32_t getFeatures() const;
};
