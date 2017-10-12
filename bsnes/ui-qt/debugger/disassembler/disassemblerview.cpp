#include "disassemblerview.moc"

// ------------------------------------------------------------------------
DisassemblerView::DisassemblerView(DisasmProcessor *processor) : hasValidAddress(false), processor(processor) {
  setFont(QFont(Style::Monospace));

  _addressAreaColor = this->palette().alternateBase().color();
  _selectionColor = this->palette().highlight().color();
  _breakpointColor = QColor(255, 0, 0, 255);

  connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onScroll()));

  init();
}

// ------------------------------------------------------------------------
void DisassemblerView::init() {
  verticalScrollBar()->setValue(0);

  emptyRowsAround = 5;
  currentRangeStartAddress = 0;
  currentRangeEndAddress = 0;
  currentAddress = 0;
}

// ------------------------------------------------------------------------
void DisassemblerView::setFont(const QFont &font) {
  QWidget::setFont(font);

  charWidth = fontMetrics().width(QLatin1Char('2'));
  charHeight = fontMetrics().height() + 1;
  lineOffset = 3;
  addressAreaSize = charWidth * 6 + charWidth;
  opcodeAreaLeft = addressAreaSize + charWidth;
  commentAreaLeft = opcodeAreaLeft + 70 * charWidth;

  adjust();
  viewport()->update();
}

// ------------------------------------------------------------------------
void DisassemblerView::adjust() {
  rowsShown = ((viewport()->height() - 4) / charHeight) + 1;
  if (rowsShown == 0) {
    rowsShown = 1;
  }
}

// ------------------------------------------------------------------------
void DisassemblerView::onScroll() {
  updateVisibleLines();
}

// ------------------------------------------------------------------------
void DisassemblerView::refresh(uint32_t address) {
  currentAddress = address;
  hasValidAddress = true;

  updateLines();
  viewport()->update();
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

  uint32_t line = topLine;
  for (uint32_t index=0; index<maxDisplayLines; index++, line++) {
    if (line < emptyRowsAround || line >= maxLines - emptyRowsAround) {
      lines[index].setEmpty();
    } else {
      processor->getLine(lines[index], address);
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
}

// ------------------------------------------------------------------------
void DisassemblerView::resizeEvent(QResizeEvent *) {
  adjust();
  updateLines();
}

// ------------------------------------------------------------------------
void DisassemblerView::mousePressEvent(QMouseEvent * event) {
  const QPoint pos = event->pos();
  int32_t row = ((pos.y() - lineOffset + charHeight) / charHeight);

  if (row < 0 || row >= lines.size()) {
    return;
  }

  const DisassemblerLine &line = lines[row];
  if (line.isEmpty()) {
    return;
  }

  if (pos.x() < addressAreaSize && line.hasAddress()) {
    int32_t breakpoint = breakpointEditor->indexOfBreakpointExec(line.address, processor->getBreakpointBusName());

    if (breakpoint >= 0) {
      breakpointEditor->removeBreakpoint(breakpoint);
    } else {
      breakpointEditor->addBreakpoint(nall::hex(line.address), "x", processor->getBreakpointBusName());
    }
  }

  viewport()->update();
}

// ------------------------------------------------------------------------
void DisassemblerView::paintOpcode(QPainter &painter, const DisassemblerLine &line, int y) {
  QString address;

  QColor addressColor, opColor, textColor;
  QColor paramImmediateColor, paramAddressColor, paramSymbolColor;

  if (line.address == currentAddress) {
    painter.fillRect(QRect(0, y - charHeight + lineOffset, viewport()->width(), charHeight), _selectionColor);

    addressColor = viewport()->palette().highlightedText().color();
    textColor = addressColor;
    opColor = addressColor;
    paramImmediateColor = addressColor;
    paramAddressColor = addressColor;
    paramSymbolColor = addressColor;
  } else {
    addressColor = Qt::gray;
    textColor = viewport()->palette().color(QPalette::WindowText);
    opColor = QColor(0x00, 0x00, 0x88, 0xff);
    paramImmediateColor = QColor(0x00, 0x88, 0x00, 0xff);
    paramAddressColor = QColor(0x88, 0x00, 0x00, 0xff);
    paramSymbolColor = QColor(0xFF, 0x00, 0x00, 0xff);
  }

  if (breakpointEditor->indexOfBreakpointExec(line.address, processor->getBreakpointBusName()) >= 0) {
    painter.fillRect(QRect(0, y - charHeight + lineOffset, addressAreaSize, charHeight), _breakpointColor);
    addressColor = Qt::white;
  }

  SymbolMap *symbols = processor->getSymbols();
  Symbol currentRow = symbols->getSymbol(line.address);

  if (currentRow.type != Symbol::INVALID) {
    painter.setPen(Qt::gray);
    painter.drawLine(0, y - charHeight + lineOffset, width(), y - charHeight + lineOffset);
    painter.setPen(paramAddressColor);
    painter.drawText(commentAreaLeft + charWidth / 2, y, currentRow.name);
  }

  int x = opcodeAreaLeft;
  address = QString("%1").arg(line.address, 6, 16, QChar('0'));

  painter.setPen(addressColor);
  painter.drawText(0, y, address);

  painter.setPen(opColor);
  painter.drawText(x, y, line.text);

  QString directComment;

  if (line.paramFormat) {
    x += (line.text.length() + 1) * charWidth;

    painter.setPen(textColor);
    int left = 0;
    int textLength = line.paramFormat.length();
    for (int i=0; i<textLength; i++) {
      if (line.paramFormat[i] == '%') {
        if (left < i) {
          painter.drawText(x, y, nall::substr(line.paramFormat, left, i - left));
          x += (i - left) * charWidth;
        }

        uint8_t argNum = line.paramFormat[i+1] - '1';
        uint8_t argType = line.paramFormat[i+2];
        uint8_t argLength = line.paramFormat[i+3] - '0';

        if (line.params.size() <= argNum) {
          painter.setPen(paramAddressColor);
          painter.drawText(x, y, "???");
          x += 3 * charWidth;
        } else {
          const DisassemblerParam &param = line.params[argNum];

          switch (param.type) {
            case DisassemblerParam::Value:
              painter.setPen(paramImmediateColor);
              x += renderValue(painter, x, y, argType, argLength, param.value);
              break;

            case DisassemblerParam::Address:
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
      painter.drawText(x, y, nall::substr(line.paramFormat, left, textLength - left));
      x += (textLength - left) * charWidth;
    }

    if (directComment.length()) {
      x += 5 * charWidth;
      int newX = opcodeAreaLeft + 25 * charWidth;
      if (newX < x) {
        newX = x;
      }

      painter.setPen(Qt::gray);
      painter.drawText(newX, y, directComment);
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
void DisassemblerView::paintEvent(QPaintEvent *event) {
  QPainter painter(viewport());

  painter.fillRect(event->rect(), viewport()->palette().color(QPalette::Base));
  painter.fillRect(QRect(0, 0, addressAreaSize, height()), _addressAreaColor);

  painter.setPen(Qt::gray);
  painter.drawLine(commentAreaLeft, event->rect().top(), commentAreaLeft, height());

  int y = 0;
  for (uint32_t index=0; index<lines.size(); index++, y+=charHeight) {
    const DisassemblerLine &line = lines[index];

    switch (line.type) {
    case DisassemblerLine::Empty:
      break;

    case DisassemblerLine::Opcode:
      paintOpcode(painter, line, y);
      break;
    }
  }
}
