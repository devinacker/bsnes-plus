#include "line.hpp"

class DisassemblerView : public QAbstractScrollArea {
  Q_OBJECT

  enum MouseState {
    NO_STATE,
    STATE_TOGGLE_BREAKPOINT,
    STATE_SET_COMMENT,
    STATE_RESIZE_COLUMN,
    STATE_RESIZING_COLUMN,
    STATE_LINE
  };

  enum {
    COLUMN_ADDRESS,
    COLUMN_DISASM,
    COLUMN_COMMENT,
    NUM_COLUMNS = 3
  };

public:
  DisassemblerView(class DisasmProcessor *processor);

  void refresh(uint32_t address);

  virtual void setFont(const QFont &font);

protected:
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *);
  void mousePressEvent(QMouseEvent * event);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void updateCurrentMousePosition();
  void showLineContextMenu(const QPoint &);

  void updateLines();
  void updateLineRange();
  void updateVisibleLines();
  void init();

private slots:
  void adjust();
  void onScroll();
  void showContextMenu(const QPoint &);
  void toggleBreakpoint();
  void setComment();
  void setSymbol();

private:
  void paintHeader(QPainter &painter);
  void paintOpcode(QPainter &painter, const DisassemblerLine &line, int y);
  int renderValue(QPainter &painter, int x, int y, uint8_t type, uint8_t size, uint32_t value);

  DisasmProcessor *processor;
  bool hasValidAddress;
  uint32_t currentAddress;
  uint32_t lineOffset;

  uint32_t charWidth;
  uint32_t charHeight;
  uint32_t charPadding;
  uint32_t rowsShown;
  uint32_t headerHeight;

  uint32_t emptyRowsAround;

  uint32_t currentRangeStartAddress;
  uint32_t currentRangeEndAddress;
  uint32_t currentRangeLineNumbers;
  uint32_t topLineAddress;
  uint32_t bottomLineAddress;

  uint32_t columnSizes[NUM_COLUMNS];
  uint32_t columnPositions[NUM_COLUMNS];
  ::nall::linear_vector<DisassemblerLine> lines;

  MouseState mouseState;
  uint32_t mouseStateValue;
  uint32_t mouseStateValue2;
  int32_t mouseX;
  int32_t mouseY;

  QColor _addressAreaColor;
  QColor _breakpointColor;
  QColor _selectionColor;
};
