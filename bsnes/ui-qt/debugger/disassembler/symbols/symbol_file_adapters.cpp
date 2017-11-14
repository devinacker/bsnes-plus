#include "symbol_file_adapters.hpp"

#include "adapters/fma_symbol_file.cpp"
#include "adapters/wla_symbol_file.cpp"

// ------------------------------------------------------------------------
SymbolFileAdapters::SymbolFileAdapters() {
  registerAdapter(new FmaSymbolFile());
  registerAdapter(new WlaSymbolFile());
}

// ------------------------------------------------------------------------
void SymbolFileAdapters::registerAdapter(SymbolFileInterface* adapter) {
  adapters.append(adapter);
}

// ------------------------------------------------------------------------
SymbolFileInterface *SymbolFileAdapters::findBestAdapter(const nall::lstring &rows) {
  SymbolFileInterface *current = NULL;
  int32_t currentScore = -1;

  for (uint32_t i=0; i<adapters.size(); i++) {
    SymbolFileInterface *adapter = adapters[i];
    uint32_t features = adapter->getFeatures();

    if ((features & SymbolFileInterface::Readable) == 0) {
      continue;
    }

    int32_t score = adapter->scoreReadString(rows);
    if (score > currentScore) {
      currentScore = score;
      current = adapter;
    }
  }

  return current;
}

// ------------------------------------------------------------------------
SymbolFileInterface *SymbolFileAdapters::fetchAdapter(uint32_t requiredFeatures, uint32_t optionalFeatures) {
  SymbolFileInterface *current = NULL;
  uint32_t currentScore = 0;

  for (uint32_t i=0; i<adapters.size(); i++) {
    SymbolFileInterface *adapter = adapters[i];
    uint32_t features = adapter->getFeatures();
    uint32_t score = 0;

    if ((features & requiredFeatures) != requiredFeatures) {
      continue;
    }

    features &= optionalFeatures;
    for (; features; features >>= 1) {
      if (features & 1) {
        score++;
      }
    }

    if (score > currentScore || current == NULL) {
      current = adapter;
      currentScore= score;
    }
  }

  return current;
}

// ------------------------------------------------------------------------
