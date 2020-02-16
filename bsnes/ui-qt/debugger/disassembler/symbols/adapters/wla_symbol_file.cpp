#include "wla_symbol_file.hpp"

// ------------------------------------------------------------------------
const char *WlaSymbolFile::getName() const {
  return "WLA symbol file";
}

// ------------------------------------------------------------------------
const char *WlaSymbolFile::getDescription() const {
  return "WLA-Assembler symbol file format";
}

// ------------------------------------------------------------------------
uint32_t WlaSymbolFile::getFeatures() const {
  return 0
    | SymbolFileInterface::Readable
    | SymbolFileInterface::Writable
    | SymbolFileInterface::Symbols
    | SymbolFileInterface::Comments
  ;
}

// ------------------------------------------------------------------------
string WlaSymbolFile::filteredRow(const string &input) const {
  string row(input);

  row.trim("\r");
  optional<unsigned> comment = row.position(";");
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
int32_t WlaSymbolFile::scoreReadString(const lstring &rows) const {
  if (rows.size() == 0) {
    return -1;
  }

  bool isInLabelsSection = false;

  for (uint32_t i=0; i<rows.size(); i++) {
    const string &row = filteredRow(rows[i]);
    if (row.length() == 0) {
      continue;
    }

    if (row == "[labels]") {
      isInLabelsSection = true;
    } else if (row[0] == '[') {
      isInLabelsSection = false;
    } else if (isInLabelsSection) {
      uint32_t address = (nall::hex(nall::substr(row, 0, 2)) << 16) | nall::hex(nall::substr(row, 3, 4));

      if (address > 0) {
        return 1;
      }
    }
  }

  return -1;
}

// ------------------------------------------------------------------------
bool WlaSymbolFile::read(const lstring &rows, SymbolMap *map) const {
  enum Section {
    SECTION_UNKNOWN,
    SECTION_LABELS,
    SECTION_COMMENTS,
  };

  Section section = SECTION_LABELS;
  for (int i=0; i<rows.size(); i++) {
    string row = filteredRow(rows[i]);
    if (row.length() == 0) {
      continue;
    }

    if (row[0] == '[') {
      if (row == "[labels]") { section = SECTION_LABELS; }
      else if (row == "[comments]") { section = SECTION_COMMENTS; }
      else { section = SECTION_UNKNOWN; }
      continue;
    }

    switch (section) {
    case SECTION_LABELS:
      map->addLocation(
        (nall::hex(nall::substr(row, 0, 2)) << 16) | nall::hex(nall::substr(row, 3, 4)),
        nall::substr(row, 8, row.length() - 8)
      );
      break;

    case SECTION_COMMENTS:
      map->addComment(
        (nall::hex(nall::substr(row, 0, 2)) << 16) | nall::hex(nall::substr(row, 3, 4)),
        nall::substr(row, 8, row.length() - 8)
      );
      break;

    case SECTION_UNKNOWN:
      break;
    }
  }

  return true;
}

// ------------------------------------------------------------------------
string WlaSymbolFile::writeAddress(uint32_t address) const {
  return string(hex<2,'0'>(address>>16), ":" , hex<4,'0'>(address&0xFFFF));
}

// ------------------------------------------------------------------------
bool WlaSymbolFile::write(nall::file &f, SymbolMap *map) const {
  uint32_t i;
  Symbol s;

  f.print("[labels]\n");
  for (i=0; i<map->symbols.size(); i++) {
    s = map->symbols[i].getSymbol();

    if (!s.isInvalid()) {
      f.print(writeAddress(s.address), " ", s.name, "\n");
    }
  }

  f.print("\n");
  f.print("[comments]\n");
  for (i=0; i<map->symbols.size(); i++) {
    s = map->symbols[i].getComment();
    if (!s.isInvalid()) {
      f.print(writeAddress(s.address), " ", s.name, "\n");
    }
  }

  return true;
}
