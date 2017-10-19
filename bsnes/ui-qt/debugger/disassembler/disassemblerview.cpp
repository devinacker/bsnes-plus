#include "disassemblerview.moc"

// ------------------------------------------------------------------------
DisassemblerView::DisassemblerView(DisasmProcessor *processor) : hasValidAddress(false), processor(processor) {
  setFont(QFont(Style::Monospace));
  setMouseTracking(true);
  setContextMenuPolicy(Qt::CustomContextMenu);

  _addressAreaColor = this->palette().alternateBase().color();
  _selectionColor = this->palette().highlight().color();
  _breakpointColor = QColor(255, 0, 0, 255);

  connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onScroll()));
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));

  init();
}

// ------------------------------------------------------------------------
void DisassemblerView::init() {
  verticalScrollBar()->setValue(0);

  mouseState = NO_STATE;
  emptyRowsAround = 5;
  currentRangeStartAddress = 0;
  currentRangeEndAddress = 0;
  currentAddress = 0;
  mouseX = 0;
  mouseY = 0;
}

// ------------------------------------------------------------------------
void DisassemblerView::setFont(const QFont &font) {
  QWidget::setFont(font);

  charWidth = fontMetrics().width(QLatin1Char('2'));
  charHeight = fontMetrics().height() + 1;
  charPadding = charWidth / 2;
  headerHeight = charHeight + 3;
  lineOffset = 3;

  columnSizes[0] = charWidth * 6 + charWidth;
  columnSizes[1] = columnSizes[0] + 30 * charWidth;

  adjust();
  viewport()->update();
}

// ------------------------------------------------------------------------
void DisassemblerView::adjust() {
  rowsShown = ((viewport()->height() - 4) / charHeight) + 1;
  if (rowsShown == 0) {
    rowsShown = 1;
  }

  columnPositions[0] = 0;
  columnPositions[1] = columnPositions[0] + columnSizes[0];
  columnPositions[2] = columnPositions[1] + columnSizes[1];
  columnSizes[2] = width() - columnPositions[2];
}

// ------------------------------------------------------------------------
void DisassemblerView::onScroll() {
  updateVisibleLines();
}

// ------------------------------------------------------------------------
void DisassemblerView::refresh(uint32_t address) {
  currentAddress = address;
  currentPcAddress = processor->getCurrentAddress();
  hasValidAddress = true;

  SNES::cpuAnalyst.performAnalysis(address, SNES::CPUAnalystState(SNES::cpu.regs.e, SNES::cpu.regs.p.m, SNES::cpu.regs.p.x), true);

  updateLines();
  viewport()->update();
  updateCurrentMousePosition();
}

// ------------------------------------------------------------------------
void DisassemblerView::createLoopUpwards(uint32_t line, uint32_t targetAddress) {
  int32_t topLine = line;

  while (topLine >= 0) {
    const RenderableDisassemblerLine &rop = lines[topLine];
    if (rop.line.address <= targetAddress) {
      // Do not highlight if we end inside another loop
      if (rop.depth > 0) {
        return;
      }

      break;
    }

    // Do not highlight if there is a return inside
    if (rop.isReturn()) {
      return;
    }

    --topLine;
  }

  int32_t currentLine = topLine;
  if (currentLine >= 0) {
    lines[currentLine].flags |= RenderableDisassemblerLine::FLAG_START_BRA;
    lines[currentLine].numStarts++;
  }

  while (++currentLine <= line) {
    lines[currentLine].depth++;
  }

  lines[line].braRelativeLine = topLine - line;
  lines[line].flags |= RenderableDisassemblerLine::FLAG_END_BRA;
}

// ------------------------------------------------------------------------
void DisassemblerView::updateVisibleLines() {
  uint32_t topLine = verticalScrollBar()->value();
  uint32_t maxLines = currentRangeLineNumbers + emptyRowsAround + emptyRowsAround;
  uint32_t address = processor->findStartLineAddress(currentRangeStartAddress, topLine < emptyRowsAround ? 0 : topLine - emptyRowsAround);

  uint32_t maxDisplayLines = rowsShown;
  if (maxLines - topLine < maxDisplayLines) {
    maxDisplayLines = maxLines - topLine;
  }

  lines.reset();
  lines.reserve(maxDisplayLines);

  bool first = true;
  uint32_t line = topLine;
  for (uint32_t index=0; index<maxDisplayLines; index++, line++) {
    if (line < emptyRowsAround || line >= maxLines - emptyRowsAround) {
      lines[index].line.setEmpty();
    } else {
      uint32_t currentAddress = address;

      if (first) {
        topLineAddress = address;
        first = false;
      }

      RenderableDisassemblerLine &rop = lines[index];
      processor->getLine(rop.line, address);
      const DisassemblerLine &op = rop.line;

      if (op.isReturn()) {
        rop.flags |= RenderableDisassemblerLine::FLAG_RETURN;
      }

      if (op.isBra()&& op.targetAddress < currentAddress && op.targetAddress >= currentRangeStartAddress) {
        createLoopUpwards(index, op.targetAddress);
      }

      bottomLineAddress = address;
    }
  }
}

// ------------------------------------------------------------------------
void DisassemblerView::updateLines() {
  if (!hasValidAddress) {
    verticalScrollBar()->setRange(0, 1);
    verticalScrollBar()->setPageStep(1);
    return;
  }

  if (currentAddress < currentRangeStartAddress || currentAddress >= currentRangeEndAddress) {
    updateLineRange();
  } else if (lines.size()) {
    if (currentAddress <= topLineAddress || currentAddress >= bottomLineAddress) {
      updateLineRange();
    }
  }

  updateVisibleLines();
}

// ------------------------------------------------------------------------
void DisassemblerView::updateLineRange() {
  uint32_t currentAddressLine;
  processor->findKnownRange(currentAddress, currentRangeStartAddress, currentRangeEndAddress, currentAddressLine, currentRangeLineNumbers);

  verticalScrollBar()->setRange(0, currentRangeLineNumbers + emptyRowsAround + emptyRowsAround - rowsShown);
  verticalScrollBar()->setPageStep(rowsShown);

  verticalScrollBar()->setValue(currentAddressLine - emptyRowsAround);

  lines.reset();
}

// ------------------------------------------------------------------------
void DisassemblerView::resizeEvent(QResizeEvent *) {
  adjust();
  updateLines();
}

// ------------------------------------------------------------------------
void DisassemblerView::setSymbol() {
  if (!processor->getSymbols()) {
    return;
  }

  uint32_t address = mouseStateValue;
  Symbol symbol = processor->getSymbols()->getSymbol(address);
  QString currentSymbol("");

  if (symbol.isSymbol()) {
    currentSymbol = (const char*)symbol.name;
  }

  bool ok;
  QString value = QInputDialog::getText(this, "Symbol", "Enter name for address " + nall::hex(address), QLineEdit::Normal, currentSymbol, &ok);
  string s;
  s = qPrintable(value);

  if (ok) {
    if (symbol.isSymbol()) {
      processor->getSymbols()->removeSymbol(address, Symbol::LOCATION);
    }

    if (s.length()) {
      processor->getSymbols()->addLocation(address, s);
    }
  }

  viewport()->update();
}

// ------------------------------------------------------------------------
void DisassemblerView::setComment() {
  if (!processor->getSymbols()) {
    return;
  }

  uint32_t address = mouseStateValue;
  Symbol comment = processor->getSymbols()->getComment(address);
  QString currentComment("");

  if (comment.isComment()) {
    currentComment = (const char*)comment.name;
  }

  bool ok;
  QString value = QInputDialog::getText(this, "Comment", "Enter comment for address " + nall::hex(address), QLineEdit::Normal, currentComment, &ok);
  string s;
  s = qPrintable(value);

  if (ok) {
    if (comment.isComment()) {
      processor->getSymbols()->removeSymbol(address, Symbol::COMMENT);
    }

    if (s.length()) {
      processor->getSymbols()->addComment(address, s);
    }
  }

  viewport()->update();
}

// ------------------------------------------------------------------------
void DisassemblerView::toggleBreakpoint() {
  uint32_t address = mouseStateValue;
  int32_t breakpoint = breakpointEditor->indexOfBreakpointExec(address, processor->getBreakpointBusName());

  if (breakpoint >= 0) {
    breakpointEditor->removeBreakpoint(breakpoint);
  } else {
    breakpointEditor->addBreakpoint(nall::hex(address), "x", processor->getBreakpointBusName());
  }

  viewport()->update();
}

// ------------------------------------------------------------------------
void DisassemblerView::mouseReleaseEvent(QMouseEvent *event) {
  switch (mouseState) {
  case STATE_RESIZING_COLUMN:
    mouseState = STATE_RESIZE_COLUMN;
    mouseMoveEvent(event);
    break;
  }
}

// ------------------------------------------------------------------------
void DisassemblerView::mousePressEvent(QMouseEvent * event) {
  bool left = event->button() & Qt::LeftButton;

  switch (mouseState) {
  case STATE_RESIZE_COLUMN:
    if (left) {
      mouseState = STATE_RESIZING_COLUMN;
      mouseStateValue2 = columnSizes[mouseStateValue];
    }
    break;

  case STATE_JUMP_TO_ADDRESS:
    if (left) {
      refresh(mouseStateValue);
    }
    break;

  case STATE_LINE:
    currentAddress = mouseStateValue;
    viewport()->update();
    break;

  case STATE_SET_COMMENT:
    if (left) {
      setComment();
    }
    break;

  case STATE_TOGGLE_BREAKPOINT:
    if (left) {
      toggleBreakpoint();
    }
    break;
  }
}

// ------------------------------------------------------------------------
void DisassemblerView::jumpToPc() {
  refresh(processor->getCurrentAddress());
}

// ------------------------------------------------------------------------
void DisassemblerView::jumpToAddress() {
  bool ok;
  QString value = QInputDialog::getText(this, "Jump to address", "Enter address to jump to", QLineEdit::Normal, hex(currentPcAddress), &ok);
  string s;
  s = qPrintable(value);

  if (ok) {
    refresh(hex(s) & (processor->getBusSize() - 1));
  }
}

// ------------------------------------------------------------------------
void DisassemblerView::showLineContextMenu(const QPoint &point) {
  QMenu contextMenu("Context menu", this);

  QAction setCommentAction("Set comment", this);
  connect(&setCommentAction, SIGNAL(triggered()), this, SLOT(setComment()));

  QAction setSymbolAction("Set symbol", this);
  connect(&setSymbolAction, SIGNAL(triggered()), this, SLOT(setSymbol()));

  QAction setBreakpoint("Toggle breakpoint", this);
  connect(&setBreakpoint, SIGNAL(triggered()), this, SLOT(toggleBreakpoint()));
  contextMenu.addAction(&setBreakpoint);

  QAction jumpToPcAction("Jump to PC", this);
  connect(&jumpToPcAction, SIGNAL(triggered()), this, SLOT(jumpToPc()));

  QAction jumpToAddressAction("Jump to address", this);
  connect(&jumpToAddressAction, SIGNAL(triggered()), this, SLOT(jumpToAddress()));

  if (processor->getSymbols() != NULL) {
    contextMenu.addAction(&setCommentAction);
    contextMenu.addAction(&setSymbolAction);
  }

  contextMenu.addSeparator();

  contextMenu.addAction(&jumpToPcAction);
  contextMenu.addAction(&jumpToAddressAction);

  contextMenu.exec(mapToGlobal(point));
}

// ------------------------------------------------------------------------
void DisassemblerView::showContextMenu(const QPoint &point) {
  switch (mouseState) {
  case STATE_SET_COMMENT:
  case STATE_TOGGLE_BREAKPOINT:
  case STATE_JUMP_TO_ADDRESS:
  case STATE_LINE:
    showLineContextMenu(point);
    break;
  }
}

// ------------------------------------------------------------------------
void DisassemblerView::mouseMoveEvent(QMouseEvent *e) {
  switch (mouseState) {
  case STATE_RESIZING_COLUMN:
    {
      int32_t newSize = mouseStateValue2 + (e->x() - mouseX);
      if (newSize < 10) {
        newSize= 10;
      }

      columnSizes[mouseStateValue] = newSize;
      adjust();
      viewport()->update();
    }
    break;

  default:
    mouseX = e->x();
    mouseY = e->y();
    updateCurrentMousePosition();
  }
}

// ------------------------------------------------------------------------
void DisassemblerView::updateCurrentMousePosition() {
  MouseState oldState = mouseState;
  int32_t row = (mouseY - lineOffset + charHeight) / charHeight;

  mouseState = NO_STATE;

  if (mouseY < headerHeight) {
    for (uint32_t i=1; i<NUM_COLUMNS; i++) {
      if (mouseX < columnPositions[i] - 5 || mouseX > columnPositions[i] + 5) {
        continue;
      }

      mouseState = STATE_RESIZE_COLUMN;
      mouseStateValue = i - 1;
      break;
    }
  } else if (row > 0 && row < lines.size()) {
    const DisassemblerLine &line = lines[row].line;

    if (!line.isEmpty()) {
      if (mouseX < columnSizes[COLUMN_ADDRESS]) {
        if (line.hasAddress()) {
          mouseState = STATE_TOGGLE_BREAKPOINT;
          mouseStateValue = line.address;
        }
      } else if (mouseX >= columnPositions[COLUMN_COMMENT]) {
        mouseState = STATE_SET_COMMENT;
        mouseStateValue = line.address;
      } else if (line.hasAddress() && line.isBra() && mouseX >= lines[row].addressPosX && mouseX < lines[row].addressPosX + lines[row].addressSizeX) {
        mouseState = STATE_JUMP_TO_ADDRESS;
        mouseStateValue = line.targetAddress;
      } else {
        mouseState = STATE_LINE;
        mouseStateValue = line.address;
      }
    }
  }

  if (mouseState != oldState) {
    switch (mouseState) {
    case STATE_RESIZE_COLUMN:
      setCursor(Qt::SplitHCursor);
      break;

    case STATE_JUMP_TO_ADDRESS:
    case STATE_TOGGLE_BREAKPOINT:
      setCursor(Qt::PointingHandCursor);
      break;

    case NO_STATE:
    default:
      unsetCursor();
      break;
    }
  }
}

#define SET_CLIPPING(region) \
  painter.setClipping(true); \
  painter.setClipRegion(QRect(columnPositions[region], 0, columnSizes[region], height()));

#define NO_CLIPPING() \
  painter.setClipping(false)

// ------------------------------------------------------------------------
void DisassemblerView::paintOpcode(QPainter &painter, RenderableDisassemblerLine &line, int y) {
  QString address;

  QColor addressColor, opColor, textColor;
  QColor paramImmediateColor, paramAddressColor, paramSymbolColor;

  NO_CLIPPING();

  if (line.line.address == currentPcAddress) {
    painter.fillRect(QRect(0, y - charHeight + lineOffset, viewport()->width(), charHeight), _selectionColor);

    addressColor = viewport()->palette().highlightedText().color();
    textColor = addressColor;
    opColor = addressColor;
    paramImmediateColor = addressColor;
    paramAddressColor = addressColor;
    paramSymbolColor = addressColor;
  } else {
    if (line.line.address == currentAddress) {
      painter.fillRect(QRect(0, y - charHeight + lineOffset, viewport()->width(), charHeight), _addressAreaColor);
    }

    addressColor = Qt::gray;
    textColor = viewport()->palette().color(QPalette::WindowText);
    opColor = QColor(0x00, 0x00, 0x88, 0xff);
    paramImmediateColor = QColor(0x00, 0x88, 0x00, 0xff);
    paramAddressColor = QColor(0x88, 0x00, 0x00, 0xff);
    paramSymbolColor = QColor(0xFF, 0x00, 0x00, 0xff);
  }

  if (breakpointEditor->indexOfBreakpointExec(line.line.address, processor->getBreakpointBusName()) >= 0) {
    painter.fillRect(QRect(0, y - charHeight + lineOffset, columnSizes[COLUMN_ADDRESS], charHeight), _breakpointColor);
    addressColor = Qt::white;
  }

  SymbolMap *symbols = processor->getSymbols();
  Symbol currentRow = symbols ? symbols->getSymbol(line.line.address) : Symbol::createInvalid();

  if (line.isReturn()) {
    painter.setPen(Qt::gray);
    painter.drawLine(0, y + lineOffset, width(), y + lineOffset);
  }

  if (currentRow.type != Symbol::INVALID) {
    painter.setPen(Qt::gray);
    painter.drawLine(0, y - charHeight + lineOffset, width(), y - charHeight + lineOffset);
    painter.setPen(paramAddressColor);
    SET_CLIPPING(COLUMN_COMMENT);
    painter.drawText(columnPositions[COLUMN_COMMENT] + charPadding, y, currentRow.name);
  } else {
    currentRow = symbols ? symbols->getComment(line.line.address) : Symbol::createInvalid();

    painter.setPen(Qt::gray);
    SET_CLIPPING(COLUMN_COMMENT);

    if (currentRow.isComment()) {
      painter.drawText(columnPositions[COLUMN_COMMENT] + charPadding, y, currentRow.name);
    } else if (line.isReturn()) {
      painter.drawText(columnPositions[COLUMN_COMMENT] + charPadding, y, "Return");
    }
  }

  int x = columnPositions[1] + charPadding + line.depth * 2 * charWidth;

  SET_CLIPPING(COLUMN_DISASM);
  if (line.isStartBra()) {
    x += line.numStarts * 2 * charWidth;
  }
  if (line.isEndBra()) {
    int l = x - charWidth - charWidth + 1;
    int r = x - charPadding;
    int w = 4;
    int hw = w >> 1;
    int ap = 2;
    int ar = r + 2;
    int al = ar - (ap + hw);
    int b = y + lineOffset + hw - (charHeight >> 1);
    int t = b - w + (line.braRelativeLine) * charHeight - 1;

    QPainterPath path;
    path.moveTo(al, t-ap);
    path.lineTo(ar, t + hw);
    path.lineTo(al, t + w + ap);

    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    painter.drawEllipse(QPoint(l + hw, b - hw), hw, hw);
    painter.drawEllipse(QPoint(l + hw, t + hw), hw, hw);
    painter.drawRect(l, t + hw, w, b - t - w);
    painter.drawRect(l + hw, b - w, r - l - hw, w);
    painter.drawRect(l + hw, t, al - l - hw, w);
    painter.drawPath(path);
  }

  address = QString("%1").arg(line.line.address, 6, 16, QChar('0'));

  SET_CLIPPING(COLUMN_ADDRESS);
  painter.setPen(addressColor);
  painter.drawText(0, y, address);

  SET_CLIPPING(COLUMN_DISASM);
  painter.setPen(opColor);
  painter.drawText(x, y, line.line.text);

  QString directComment;

  if (line.line.paramFormat) {
    x += (line.line.text.length() + 1) * charWidth;

    painter.setPen(textColor);
    int left = 0;
    int textLength = line.line.paramFormat.length();
    for (int i=0; i<textLength; i++) {
      if (line.line.paramFormat[i] == '%') {
        if (left < i) {
          painter.drawText(x, y, nall::substr(line.line.paramFormat, left, i - left));
          x += (i - left) * charWidth;
        }

        uint8_t argNum = line.line.paramFormat[i+1] - '1';
        uint8_t argType = line.line.paramFormat[i+2];
        uint8_t argLength = line.line.paramFormat[i+3] - '0';

        if (line.line.params.size() <= argNum) {
          painter.setPen(paramAddressColor);
          painter.drawText(x, y, "???");
          x += 3 * charWidth;
        } else {
          const DisassemblerParam &param = line.line.params[argNum];

          switch (param.type) {
            case DisassemblerParam::Value:
              painter.setPen(paramImmediateColor);
              x += renderValue(painter, x, y, argType, argLength, param.value);
              break;

            case DisassemblerParam::Address:
              line.addressPosX = x;
              if (symbols) {
                Symbol sym = symbols->getSymbol(param.address);

                if (sym.type != Symbol::INVALID) {
                  QString text = QString("<%1>").arg(sym.name);
                  painter.setPen(paramSymbolColor);
                  painter.drawText(x, y, text);
                  x += text.length() * charWidth;
                } else {
                  painter.setPen(paramAddressColor);
                  x += renderValue(painter, x, y, argType, argLength, param.value);
                }
              } else {
                painter.setPen(paramAddressColor);
                x += renderValue(painter, x, y, argType, argLength, param.value);
              }
              line.addressSizeX = x - line.addressPosX;
              directComment += QString("[%1]").arg(param.address, 6, 16, QChar('0'));
              break;

            default:
              painter.drawText(x, y, "???");
              x += 3 * charWidth;
              break;

          }
        }
        painter.setPen(opColor);

        i += 3;
        left = i + 1;

      }
    }

    if (left + 1 < textLength) {
      painter.drawText(x, y, nall::substr(line.line.paramFormat, left, textLength - left));
      x += (textLength - left) * charWidth;
    }

    if (directComment.length()) {
      x += 2 * charWidth;
      uint32_t right = columnPositions[COLUMN_DISASM + 1] - (directComment.length() + 1) * charWidth;

      if (x < right) {
        x = right;
      }

      painter.setPen(Qt::gray);
      painter.drawText(x, y, directComment);
    }
  }
}

// ------------------------------------------------------------------------
int DisassemblerView::renderValue(QPainter &painter, int x, int y, uint8_t type, uint8_t size, uint32_t value) {
  QString text;

  switch (type) {
    case 'X':
      text = QString("$%1").arg(value, size, 16, QChar('0'));
      break;

    default:
      text = "???";
      break;
  }

  painter.drawText(x, y, text);
  return text.length() * charWidth;
}

// ------------------------------------------------------------------------
void DisassemblerView::paintHeader(QPainter &painter) {
  painter.fillRect(0, 0, width(), headerHeight, _addressAreaColor);

  painter.setPen(Qt::gray);
  painter.drawLine(0, headerHeight, width(), headerHeight);

  for (uint8_t i=1; i<NUM_COLUMNS; i++) {
    painter.drawLine(columnPositions[i], 0, columnPositions[i], headerHeight);
  }

  painter.setPen(Qt::black);
  SET_CLIPPING(1);
  painter.drawText(columnPositions[1] + charPadding, headerHeight - charPadding, "Disassemble");
  SET_CLIPPING(2);
  painter.drawText(columnPositions[2] + charPadding, headerHeight - charPadding, "Comment");
  NO_CLIPPING();
}

// ------------------------------------------------------------------------
void DisassemblerView::paintEvent(QPaintEvent *event) {
  QPainter painter(viewport());

  painter.fillRect(event->rect(), viewport()->palette().color(QPalette::Base));
  painter.fillRect(QRect(0, 0, columnSizes[COLUMN_ADDRESS], height()), _addressAreaColor);

  painter.setPen(Qt::gray);
  painter.drawLine(columnPositions[2], event->rect().top(), columnPositions[2], height());

  int y = 0;
  for (uint32_t index=0; index<lines.size(); index++, y+=charHeight) {
    RenderableDisassemblerLine &line = lines[index];

    switch (line.line.type) {
    case DisassemblerLine::Empty:
      break;

    case DisassemblerLine::Opcode:
      paintOpcode(painter, line, y);
      break;
    }
  }

  NO_CLIPPING();
  paintHeader(painter);
}
