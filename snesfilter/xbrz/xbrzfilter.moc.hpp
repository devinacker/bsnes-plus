class XbrzFilter : public QObject {
  Q_OBJECT

public:
  void bind(configuration&);
  void size(unsigned&, unsigned&, unsigned, unsigned);
  void render(uint32_t*, unsigned, const uint16_t*, unsigned, unsigned, unsigned);
  QWidget *settings();

private:
  QWidget *widget = nullptr;
  
private:
  unsigned factor;
  bool nearestNeighbor;
  std::vector<uint32_t> src_32;

private slots:
  void factorChanged(int value);
  void nearestNeighborChanged(int value);
} filter_xbrz;

