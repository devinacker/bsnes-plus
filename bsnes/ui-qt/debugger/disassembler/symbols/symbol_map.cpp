#include "symbol_map.moc"

// ------------------------------------------------------------------------
Symbol Symbols::getSymbol() {
  for (uint32_t i=0; i<symbols.size(); i++) {
    if (symbols[i].isSymbol()) {
      return symbols[i];
    }
  }

  return Symbol::createInvalid();
}

  // ------------------------------------------------------------------------
Symbol Symbols::getComment() {
  for (uint32_t i=0; i<symbols.size(); i++) {
    if (symbols[i].isComment()) {
      return symbols[i];
    }
  }

  return Symbol::createInvalid();
}

// ------------------------------------------------------------------------
SymbolMap::SymbolMap() {
  isValid = false;
}

// ------------------------------------------------------------------------
int32_t SymbolMap::getSymbolIndex(uint32_t address) {
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
      return cur;
    }
  }

  return -1;
}

// ------------------------------------------------------------------------
void SymbolMap::addLocation(uint32_t address, const string &name) {
  addSymbol(address, Symbol::createLocation(address, name));
}

// ------------------------------------------------------------------------
void SymbolMap::addComment(uint32_t address, const string &name) {
  addSymbol(address, Symbol::createComment(address, name));
}

// ------------------------------------------------------------------------
void SymbolMap::addSymbol(uint32_t address, const Symbol &name) {
  int32_t index = getSymbolIndex(address);
  if (index == -1) {
    Symbols s;
    s.address = address;
    s.symbols.append(Symbol(name));
    symbols.append(s);
    isValid = false;
  } else {
    symbols[index].symbols.append(Symbol(name));
  }
}

// ------------------------------------------------------------------------
void SymbolMap::finishUpdates() {
  emit updated();
}

// ------------------------------------------------------------------------
void SymbolMap::revalidate() {
  if (!isValid) {
    symbols.sort();
    isValid = true;
  }
}

// ------------------------------------------------------------------------
Symbol SymbolMap::getSymbol(uint32_t address) {
  int32_t index = getSymbolIndex(address);
  if (index == -1) {
    return Symbol::createInvalid();
  }

  return symbols[index].getSymbol();
}

// ------------------------------------------------------------------------
Symbol SymbolMap::getComment(uint32_t address) {
  int32_t index = getSymbolIndex(address);
  if (index == -1) {
    return Symbol::createInvalid();
  }

  return symbols[index].getComment();
}

// ------------------------------------------------------------------------
void SymbolMap::removeSymbol(uint32_t address, Symbol::Type type) {
  int32_t index = getSymbolIndex(address);
  if (index == -1) {
    return;
  }

  Symbols &s = symbols[index];
  for (int32_t i=0; i<s.symbols.size(); i++) {
    if (s.symbols[i].type == type) {
      s.symbols.remove(i);
      i--;
    }
  }

  if (s.symbols.size() == 0) {
    symbols.remove(index);
  }
}

// ------------------------------------------------------------------------
void SymbolMap::saveToFile(const string &baseName, const string &ext) {
  revalidate();

  if (symbols.size() == 0) {
    return;
  }

  string fileName = baseName;
  fileName.append(ext);

  ::nall::file f;
  uint32_t i;
  Symbol s;
  if (!f.open((const char*)fileName, ::nall::file::mode::write)) {
    return;
  }

  f.print("[labels]\n");
  for (i=0; i<symbols.size(); i++) {
    s = symbols[i].getSymbol();
    if (!s.isInvalid()) {
      f.print(hex<2,'0'>(s.address>>16), ":" , hex<4,'0'>(s.address&0xFFFF), " ", s.name, "\n");
    }
  }

  f.print("\n[comments]\n");
  for (i=0; i<symbols.size(); i++) {
    s = symbols[i].getComment();
    if (!s.isInvalid()) {
      f.print(hex<2,'0'>(s.address>>16), ":" , hex<4,'0'>(s.address&0xFFFF), " ", s.name, "\n");
    }
  }

  f.close();
}

// ------------------------------------------------------------------------
void SymbolMap::loadFromFile(const string &baseName, const string &ext) {
  string fileName = baseName;
  fileName.append(ext);

  string buffer;
  if (buffer.readfile(fileName)) {
    loadFromString(buffer);
  }
}

// ------------------------------------------------------------------------
void SymbolMap::loadFromString(const string &file) {
  nall::lstring rows;
  rows.split("\n", file);

  enum Section {
    SECTION_UNKNOWN,
    SECTION_LABELS,
    SECTION_COMMENTS,
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
      else if (row == "[comments]") { section = SECTION_COMMENTS; }
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

    case SECTION_COMMENTS:
      addComment(
        (nall::hex(nall::substr(row, 0, 2)) << 16) | nall::hex(nall::substr(row, 3, 4)),
        nall::substr(row, 8, row.length() - 8)
      );
      break;

    case SECTION_UNKNOWN:
      break;
    }
  }

  finishUpdates();
}

// ------------------------------------------------------------------------
