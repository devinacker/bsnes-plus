#include "symbol_map.hpp"

// ------------------------------------------------------------------------
SymbolMap::SymbolMap() {
  isValid = false;
}

// ------------------------------------------------------------------------
void SymbolMap::addLocation(uint32_t address, const string &name) {
  symbols.append(Symbol::createLocation(address, name));
  isValid = false;
}

// ------------------------------------------------------------------------
void SymbolMap::revalidate() {
  if (isValid) {
    return;
  }

  // Don't know how to do this with pure nall stuff :(
  int numSymbols = symbols.size();
  Symbol *temp = new Symbol[numSymbols];
  for (int i=0; i<numSymbols; i++) {
    temp[i] = symbols[i];
  }

  nall::sort(temp, numSymbols);

  symbols.reset();
  symbols.reserve(numSymbols);
  for (int i=0; i<numSymbols; i++) {
    symbols.append(temp[i]);
  }

  isValid = true;
}

// ------------------------------------------------------------------------
Symbol SymbolMap::getSymbol(uint32_t address) {
  revalidate();

  int32_t left = 0;
  int32_t right = symbols.size() - 1;

  while (right >= left) {
    uint32_t cur = ((right - left) >> 1) + left;
    uint32_t curaddr = symbols[cur].address;

    if (address < curaddr) {
      right = cur - 1;
    } else if (address > curaddr) {
      left = cur + 1;
    } else {
      return symbols[cur];
    }
  }

  return Symbol::createInvalid();
}

// ------------------------------------------------------------------------
void SymbolMap::loadFromFile(const string &baseName, const string &ext) {
  string fileName = baseName;
  fileName.append(ext);

  ::nall::file f;
  if (!f.open((const char*)fileName, ::nall::file::mode::read)) {
    return;
  }

  int size = f.size();
  char *buffer = new char[size + 1];
  buffer[size] = 0;
  f.read((uint8_t*)buffer, f.size());
  loadFromString(buffer);

  delete[] buffer;

  f.close();
}

// ------------------------------------------------------------------------
void SymbolMap::loadFromString(const string &file) {
  nall::lstring rows;
  rows.split("\n", file);

  enum Section {
    SECTION_UNKNOWN,
    SECTION_LABELS
  };

  Section section = SECTION_LABELS;

  for (int i=0; i<rows.size(); i++) {
    string row = rows[i];
    row.trim("\r");

    optional<unsigned> comment = row.position(";");
    if (comment) {
      unsigned index = comment();
      if (index == 0) {
        continue;
      }
      row = nall::substr(row, 0, index);
    }

    row.trim(" ");
    if (row.length() == 0) {
      continue;
    }

    if (row[0] == '[') {
      if (row == "[labels]") { section = SECTION_LABELS; }
      else { section = SECTION_UNKNOWN; }
      continue;
    }

    switch (section) {
    case SECTION_LABELS:
      addLocation(
        (nall::hex(nall::substr(row, 0, 2)) << 16) | nall::hex(nall::substr(row, 3, 4)),
        nall::substr(row, 8, row.length() - 8)
      );
      break;

    case SECTION_UNKNOWN:
      break;
    }
  }
}

// ------------------------------------------------------------------------
