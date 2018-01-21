#include "fma_symbol_file.hpp"

// ------------------------------------------------------------------------
const char *FmaSymbolFile::getName() const {
  return "FMA symbol file";
}

// ------------------------------------------------------------------------
const char *FmaSymbolFile::getDescription() const {
  return "Visit https://github.com/BenjaminSchulte/fma-snes65816/blob/master/docs/symbols.adoc for specifications.";
}

// ------------------------------------------------------------------------
uint32_t FmaSymbolFile::getFeatures() const {
  return 0
    | SymbolFileInterface::Readable
    | SymbolFileInterface::Writable
    | SymbolFileInterface::Symbols
    | SymbolFileInterface::Comments
  ;
}

// ------------------------------------------------------------------------
int32_t FmaSymbolFile::scoreReadString(const nall::lstring &rows) const {
  if (rows.size() == 0) {
    return -1;
  }

  if (rows[0] == "#SNES65816") {
    // It's our header, so we should be safe with it, shouldn't we?
    return 100;
  }

  return -1;
}

// ------------------------------------------------------------------------
string FmaSymbolFile::filteredRow(const string &input) const {
  string row(input);

  row.trim("\r");
  optional<unsigned> comment = row.position("#");
  if (comment) {
    unsigned index = comment();
    if (index == 0) {
      return "";
    }
    row = nall::substr(row, 0, index);
  }

  row.trim(" ");
  return row;
}

// ------------------------------------------------------------------------
uint32_t FmaSymbolFile::parseAddress(const string &arg) const {
  if (arg.length() != 7 || arg[2] != ':') {
    return 0;
  }

  return (nall::hex(nall::substr(arg, 0, 2)) << 16) | nall::hex(nall::substr(arg, 3, 4));
}

// ------------------------------------------------------------------------
void FmaSymbolFile::parseArgs(const nall::string &row, nall::lstring &args) const {
  args.qsplit(" ", row);

  for (uint32_t i=0; i<args.size(); i++) {
    string &arg = args[i];

    if(strbegin(arg, "\"") && strend(arg, "\"")) arg.trim<1>("\"");
    if(strbegin(arg, "\'") && strend(arg, "\'")) arg.trim<1>("\"");
  }
}

// ------------------------------------------------------------------------
void FmaSymbolFile::readComment(SymbolMap *map, const lstring &args) const {
  if (args.size() < 2) {
    return;
  }

  uint32_t address = parseAddress(args[0]);
  const string &name = args[1];

  map->addSymbol(address, Symbol::createComment(address, name));
}

// ------------------------------------------------------------------------
void FmaSymbolFile::readSymbol(SymbolMap *map, const lstring &args) const {
  if (args.size() < 4) {
    return;
  }

  uint32_t address = parseAddress(args[0]);
  const string &name = args[1];
  const string &type = args[2];
  uint32_t size = hex(args[3]);

  map->addSymbol(address, Symbol::createLocation(address, name));
}

// ------------------------------------------------------------------------
bool FmaSymbolFile::read(const nall::lstring &rows, SymbolMap *map) const {
  enum Section {
    SECTION_UNKNOWN,
    SECTION_SYMBOL,
    SECTION_FILE,
    SECTION_SOURCEMAP,
    SECTION_COMMENT,
  };

  Section section = SECTION_UNKNOWN;

  lstring args;

  for (int i=0; i<rows.size(); i++) {
    string row = filteredRow(rows[i]);

    if (row.length() == 0) {
      continue;
    }

    if (row[0] == '[') {
      if (row == "[SYMBOL]") { section = SECTION_SYMBOL; }
      else if (row == "[COMMENT]") { section = SECTION_COMMENT; }
      else if (row == "[FILE]") { section = SECTION_FILE; }
      else if (row == "[SOURCEMAP]") { section = SECTION_SOURCEMAP; }
      else { section = SECTION_UNKNOWN; }
      continue;
    }

    parseArgs(row, args);

    switch (section) {
    case SECTION_SYMBOL:
      readSymbol(map, args);
      break;

    case SECTION_COMMENT:
      readComment(map, args);
      break;

    default:
      // Not supported, yet :(
      break;
    }
  }

  return true;
};

// ------------------------------------------------------------------------
string FmaSymbolFile::writeAddress(uint32_t address) const {
  return string(hex<2,'0'>(address>>16), ":" , hex<4,'0'>(address&0xFFFF));
}

// ------------------------------------------------------------------------
bool FmaSymbolFile::write(nall::file &f, SymbolMap *map) const {
  uint32_t i;
  Symbol s;

  f.print("#SNES65816\n");
  f.print("#Visit https://github.com/BenjaminSchulte/fma-snes65816/blob/master/docs/symbols.adoc for specifications.\n");
  f.print("\n");

  f.print("[SYMBOL]\n");
  for (i=0; i<map->symbols.size(); i++) {
    s = map->symbols[i].getSymbol();

    if (!s.isInvalid()) {
      f.print(writeAddress(s.address), " ", s.name, " ANY 1\n");
    }
  }

  f.print("\n");
  f.print("[COMMENT]\n");
  for (i=0; i<map->symbols.size(); i++) {
    s = map->symbols[i].getComment();
    if (!s.isInvalid()) {
      f.print(writeAddress(s.address), " \"", s.name, "\"\n");
    }
  }

  return true;
}
