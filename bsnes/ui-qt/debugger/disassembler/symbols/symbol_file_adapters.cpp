#include "symbol_file_adapters.hpp"

#include "adapters/fma_symbol_file.cpp"
#include "adapters/wla_symbol_file.cpp"
#include "adapters/vice_label_file.cpp"

// ------------------------------------------------------------------------
SymbolFileAdapters::SymbolFileAdapters() {
  registerAdapter(new ViceLabelFile());
  registerAdapter(new WlaSymbolFile());
  registerAdapter(new FmaSymbolFile());
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
SymbolFileInterface *SymbolFileAdapters::fetchAdapter(uint32_t requiredFeatures, uint32_t optionalFeatures,
                                                      SymbolFileInterface *current) {
  uint32_t currentScore = 0;
  if (current != NULL) {
    // prioritize the adapter currently in use if it has all the features we need
	currentScore = scoreFeatures(current, requiredFeatures, optionalFeatures);
  }

  for (uint32_t i=0; i<adapters.size(); i++) {
    SymbolFileInterface *adapter = adapters[i];
    uint32_t score = scoreFeatures(adapter, requiredFeatures, optionalFeatures);

    if (score > currentScore || current == NULL) {
      current = adapter;
      currentScore= score;
    }
  }

  return current;
}

// ------------------------------------------------------------------------
uint32_t SymbolFileAdapters::scoreFeatures(const SymbolFileInterface *adapter,
                                           uint32_t requiredFeatures, uint32_t optionalFeatures) const {
  uint32_t features = adapter->getFeatures();
  uint32_t score = 0;

  if ((features & requiredFeatures) != requiredFeatures) {
    return 0;
  }

  features &= optionalFeatures;
  for (; features; features >>= 1) {
    if (features & 1) {
      score++;
    }
  }

  return score;
}

// ------------------------------------------------------------------------
