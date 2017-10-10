#ifndef __SYMBOL_MAP__H__
#define __SYMBOL_MAP__H__

struct Symbol {
  enum Type { INVALID, LOCATION };

  static Symbol createInvalid() {
    Symbol s;
    s.type = INVALID;
    return s;
  }

  static Symbol createLocation(uint32_t address, const string &name) {
    Symbol s;
    s.type = LOCATION;
    s.address = address;
    s.name = name;
    return s;
  }

  bool operator <(const Symbol &other) {
    return address < other.address;
  }

  uint32_t address;
  string name;
  Type type;
};

class SymbolMap {
public:
  SymbolMap();

  void addLocation(uint32_t address, const string &name);
  void loadFromString(const string &file);
  void loadFromFile(const string &baseName, const string &ext);

  void revalidate();

  Symbol getSymbol(uint32_t address);

  bool isValid;
  nall::linear_vector<Symbol> symbols;
};

#endif
