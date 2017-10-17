#ifndef __SYMBOL_MAP__H__
#define __SYMBOL_MAP__H__

struct Symbol {
  enum Type { INVALID, LOCATION, COMMENT };

  static Symbol createInvalid() {
    Symbol s;
    s.type = INVALID;
    return s;
  }

  static Symbol createComment(uint32_t address, const string &name) {
    Symbol s;
    s.type = COMMENT;
    s.address = address;
    s.name = name;
    return s;
  }

  static Symbol createLocation(uint32_t address, const string &name) {
    Symbol s;
    s.type = LOCATION;
    s.address = address;
    s.name = name;
    return s;
  }

  inline bool isInvalid() const {
    return type == INVALID;
  }

  inline bool isSymbol() const {
    return type == LOCATION;
  }

  inline bool isComment() const {
    return type == COMMENT;
  }

  bool operator <(const Symbol &other) {
    return address < other.address;
  }

  uint32_t address;
  string name;
  Type type;
};

struct Symbols {
  typedef nall::linear_vector<Symbol> SymbolList;

  uint32_t address;
  SymbolList symbols;

  Symbol getSymbol();
  Symbol getComment();

  bool operator <(const Symbols &other) {
    return address < other.address;
  }
};

class SymbolMap : public QObject {
  Q_OBJECT

public:
  SymbolMap();

  typedef nall::linear_vector<Symbols> SymbolsLists;

  void addLocation(uint32_t address, const string &name);
  void addComment(uint32_t address, const string &name);
  void addSymbol(uint32_t address, const Symbol &name);
  void removeSymbol(uint32_t address, Symbol::Type type);
  void loadFromString(const string &file);
  void loadFromFile(const string &baseName, const string &ext);
  void saveToFile(const string &baseName, const string &ext);
  void finishUpdates();

  void revalidate();

  int32_t getSymbolIndex(uint32_t address);
  Symbol getSymbol(uint32_t address);
  Symbol getComment(uint32_t address);

  bool isValid;
  SymbolsLists symbols;

signals:
  void updated();
};

#endif
