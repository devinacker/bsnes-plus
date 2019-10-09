class SymbolFileInterface;

class SymbolFileAdapters {
public:
  SymbolFileAdapters();

  void registerAdapter(SymbolFileInterface*);

  SymbolFileInterface *findBestAdapter(const nall::lstring &rows);
  SymbolFileInterface *fetchAdapter(uint32_t requiredFeatures, uint32_t optionalFeatures=0);

protected:
  nall::linear_vector<SymbolFileInterface*> adapters;
};
