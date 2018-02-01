#include "symbol_map.moc"
#include "symbol_file_adapters.cpp"

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
  adapters = new SymbolFileAdapters();
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
  isValid = false;

  int32_t right = symbols.size();
  for (int32_t i=0; i<right; i++) {
    if (symbols[i].address == address) {
      symbols[i].symbols.append(Symbol(name));
      return;
    }
  }


  Symbols s;
  s.address = address;
  s.symbols.append(Symbol(name));
  symbols.append(s);
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

  SymbolFileInterface *adapter = adapters->fetchAdapter(
    SymbolFileInterface::Writable,
    0
    | SymbolFileInterface::Symbols
    | SymbolFileInterface::Comments
    | SymbolFileInterface::DebugInterface
    | SymbolFileInterface::Files
    | SymbolFileInterface::LineMap
  );

  string fileName = baseName;
  fileName.append(ext);

  ::nall::file f;
  if (!f.open((const char*)fileName, ::nall::file::mode::write)) {
    return;
  }

  adapter->write(f, this);

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

  SymbolFileInterface *adapter = adapters->findBestAdapter(rows);
  if (adapter == NULL) {
    return;
  }

  if (adapter->read(rows, this)) {
    finishUpdates();
  }
}

// ------------------------------------------------------------------------
