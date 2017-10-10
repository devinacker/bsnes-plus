#include "disassemblerview.moc"

// ------------------------------------------------------------------------
DisassemblerView::DisassemblerView(DisasmProcessor *processor) : hasValidAddress(false), processor(processor) {
  setFont(QFont(Style::Monospace));

  _addressAreaColor = this->palette().alternateBase().color();
  _selectionColor = this->palette().highlight().color();

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
  addressAreaSize = charWidth * 6 + charWidth;
  opcodeAreaLeft = addressAreaSize + charWidth;

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
void DisassemblerView::paintOpcode(QPainter &painter, const DisassemblerLine &line, int y) {
  QString address;

  if (line.address == currentAddress) {
    painter.fillRect(QRect(0, y - charHeight + 3, viewport()->width(), charHeight), _selectionColor);
    painter.setPen(viewport()->palette().highlightedText().color());
  } else {
    painter.setPen(viewport()->palette().color(QPalette::WindowText));
  }


  address = QString("%1").arg(line.address, 6, 16, QChar('0'));
  painter.drawText(0, y, address);
  painter.drawText(opcodeAreaLeft, y, line.text);
}

// ------------------------------------------------------------------------
void DisassemblerView::paintEvent(QPaintEvent *event) {
  QPainter painter(viewport());

  painter.fillRect(event->rect(), viewport()->palette().color(QPalette::Base));
  painter.fillRect(QRect(0, 0, addressAreaSize, height()), _addressAreaColor);

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
