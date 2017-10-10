#include "line.hpp"

class DisassemblerView : public QAbstractScrollArea {
  Q_OBJECT

public:
  DisassemblerView(class DisasmProcessor *processor);

  void refresh(uint32_t address);

  virtual void setFont(const QFont &font);

protected:
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *);

  void updateLines();
  void updateLineRange();
  void updateVisibleLines();
  void init();

private slots:
  void adjust();
  void onScroll();

private:
  void paintOpcode(QPainter &painter, const DisassemblerLine &line, int y);

  DisasmProcessor *processor;
  bool hasValidAddress;
  uint32_t currentAddress;

  uint32_t charWidth;
  uint32_t charHeight;
  uint32_t rowsShown;
  uint32_t addressAreaSize;
  uint32_t opcodeAreaLeft;

  uint32_t emptyRowsAround;

  uint32_t currentRangeStartAddress;
  uint32_t currentRangeEndAddress;
  uint32_t currentRangeLineNumbers;

  ::nall::linear_vector<DisassemblerLine> lines;

  QColor _addressAreaColor;
  QColor _selectionColor;
};
