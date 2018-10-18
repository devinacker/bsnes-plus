#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>

#include "qhexedit.moc.hpp"

const int HEXCHARS_IN_LINE = 47;
const int BYTES_PER_LINE = 16;


// ********************************************************************** Constructor, destructor

QHexEdit::QHexEdit(QWidget *parent) : QAbstractScrollArea(parent)
{
    _undoStack = new UndoStack(this);
    setFont(QFont(Style::Monospace));
    
    setAddressAreaColor(this->palette().alternateBase().color());
    setHighlightingColor(QColor(0xff, 0xff, 0x99, 0xff));
    setSelectionColor(this->palette().highlight().color());

    connect(&_cursorTimer, SIGNAL(timeout()), this, SLOT(updateCursor()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(adjust()));
    connect(_undoStack, SIGNAL(indexChanged(int)), this, SLOT(dataChangedPrivate(int)));

    _cursorTimer.setInterval(500);
    _cursorTimer.start();

    setAddressWidth(4);
    setAddressArea(true);
    setAsciiArea(true);
    setHighlighting(true);
    setReadOnly(false);

    init();

}

QHexEdit::~QHexEdit()
{
}

// ********************************************************************** Properties

void QHexEdit::setAddressArea(bool addressArea)
{
    _addressArea = addressArea;
    adjust();
    setCursorPosition(_cursorPosition);
    viewport()->update();
}

bool QHexEdit::addressArea()
{
    return _addressArea;
}

void QHexEdit::setAddressAreaColor(const QColor &color)
{
    _addressAreaColor = color;
    viewport()->update();
}

QColor QHexEdit::addressAreaColor()
{
    return _addressAreaColor;
}

void QHexEdit::setAddressOffset(qint64 addressOffset)
{
    _addressOffset = addressOffset;
    adjust();
    setCursorPosition(_cursorPosition);
    viewport()->update();
}

qint64 QHexEdit::addressOffset()
{
    return _addressOffset;
}

void QHexEdit::setEditorSize(qint64 size)
{
    _editorSize = size;
    adjust();
    setCursorPosition(_cursorPosition);
    viewport()->update();
}

qint64 QHexEdit::editorSize()
{
    return _editorSize;
}

void QHexEdit::setAddressWidth(int addressWidth)
{
    _addressWidth = addressWidth;
    adjust();
    setCursorPosition(_cursorPosition);
    viewport()->update();
}

int QHexEdit::addressWidth()
{
    qint64 size = _editorSize;
    int n = 1;
    if (size > Q_INT64_C(0x100000000)){ n += 8; size /= Q_INT64_C(0x100000000);}
    if (size > 0x10000){ n += 4; size /= 0x10000;}
    if (size > 0x100){ n += 2; size /= 0x100;}
    if (size > 0x10){ n += 1; size /= 0x10;}

    if (n > _addressWidth)
        return n;
    else
        return _addressWidth;
}

void QHexEdit::setAsciiArea(bool asciiArea)
{
    _asciiArea = asciiArea;
    viewport()->update();
}

bool QHexEdit::asciiArea()
{
    return _asciiArea;
}

void QHexEdit::setCursorPosition(qint64 position)
{
    // 1. delete old cursor
    _blink = false;
    viewport()->update(_cursorRect);

    // 2. Check, if cursor in range?
    if (position > (_editorSize * 2 - 1))
        position = _editorSize * 2 - 1;
    if (position < 0)
        position = 0;

    // 3. Calc new position of curser
    _cursorPosition = position;
    _bPosCurrent = position / 2;
    _pxCursorY = ((position/2 - _bPosFirst) / BYTES_PER_LINE + 1) * _pxCharHeight;
    int x = (position % (2 * BYTES_PER_LINE));
    _pxCursorX = (((x / 2) * 3) + (x % 2)) * _pxCharWidth + _pxPosHexX;

    _cursorRect = QRect(_pxCursorX, _pxCursorY + _pxCursorWidth, _pxCharWidth, _pxCursorWidth);
    
    // 4. Immiadately draw new cursor
    _blink = true;
    viewport()->update(_cursorRect);
    emit currentAddressChanged(_bPosCurrent);
}

qint64 QHexEdit::cursorPosition(QPoint pos)
{
    // Calc cursorposition depending on a graphical position
    qint64 result = -1;
    if ((pos.x() >= _pxPosHexX) and (pos.x() < (_pxPosHexX + (1 + HEXCHARS_IN_LINE) * _pxCharWidth)))
    {
        int x = (pos.x() - _pxPosHexX - _pxCharWidth / 2) / _pxCharWidth;
        x = (x / 3) * 2 + x % 3;
        int y = ((pos.y() - 3) / _pxCharHeight) * 2 * BYTES_PER_LINE;
        result = _bPosFirst * 2 + x + y;
    }
    return result;
}

qint64 QHexEdit::cursorPosition()
{
    return _cursorPosition;
}

void QHexEdit::setHighlighting(bool highlighting)
{
    _highlighting = highlighting;
    viewport()->update();
}

bool QHexEdit::highlighting()
{
    return _highlighting;
}

void QHexEdit::setHighlightingColor(const QColor &color)
{
    _brushHighlighted = QBrush(color);
    _penHighlighted = QPen(viewport()->palette().color(QPalette::WindowText));
    viewport()->update();
}

QColor QHexEdit::highlightingColor()
{
    return _brushHighlighted.color();
}

void QHexEdit::setSelectionColor(const QColor &color)
{
    _brushSelection = QBrush(color);
    _penSelection = QPen(Qt::white);
    viewport()->update();
}

QColor QHexEdit::selectionColor()
{
    return _brushSelection.color();
}

bool QHexEdit::isReadOnly()
{
    return _readOnly;
}

void QHexEdit::setReadOnly(bool readOnly)
{
    _readOnly = readOnly;
}

// ********************************************************************** Char handling
void QHexEdit::insert(qint64 index, char ch)
{
    _undoStack->insert(index, ch);
    refresh();
}

void QHexEdit::remove(qint64 index, qint64 len)
{
    _undoStack->removeAt(index, len);
    refresh();
}

void QHexEdit::replace(qint64 index, char ch)
{
    _undoStack->overwrite(index, ch);
    refresh();
}

// ********************************************************************** ByteArray handling
void QHexEdit::insert(qint64 pos, const QByteArray &ba)
{
    _undoStack->insert(pos, ba);
    refresh();
}

void QHexEdit::replace(qint64 pos, qint64 len, const QByteArray &ba)
{
    _undoStack->overwrite(pos, len, ba);
    refresh();
}

// ********************************************************************** Utility functions
void QHexEdit::ensureVisible()
{
    if (_cursorPosition < (_bPosFirst * 2))
        verticalScrollBar()->setValue((int)(_cursorPosition / 2 / BYTES_PER_LINE));
    if (_cursorPosition > ((_bPosFirst + (_rowsShown - 1)*BYTES_PER_LINE) * 2))
        verticalScrollBar()->setValue((int)(_cursorPosition / 2 / BYTES_PER_LINE) - _rowsShown + 1);
    viewport()->update();
}

#define BUFFER_SIZE 0x10000

QByteArray QHexEdit::getBuffer(qint64 pos, qint64 size)
{
    QByteArray buffer;

    for (qint64 i = pos; i >= 0 && i < _editorSize && i < pos + size; i++)
        buffer.append(reader(i));
        
    return buffer;
}

qint64 QHexEdit::indexOf(const QByteArray &ba, qint64 from)
{
    qint64 result = -1;
    QByteArray buffer;

    for (qint64 pos=from; pos < _editorSize; pos += BUFFER_SIZE)
    {
        buffer = getBuffer(pos, BUFFER_SIZE + ba.size() - 1);
        int findPos = buffer.indexOf(ba);
        if (findPos >= 0)
        {
            result = pos + (qint64)findPos;
            break;
        }
    }
    return result;
}

qint64 QHexEdit::lastIndexOf(const QByteArray &ba, qint64 from)
{
    qint64 result = -1;
    QByteArray buffer;

    for (qint64 pos=from; pos > 0; pos -= BUFFER_SIZE)
    {
        qint64 sPos = pos - BUFFER_SIZE - (qint64)ba.size() + 1;
        if (sPos < 0)
            sPos = 0;
        buffer = getBuffer(sPos, pos - sPos);
        int findPos = buffer.lastIndexOf(ba);
        if (findPos >= 0)
        {
            result = sPos + (qint64)findPos;
            break;
        }
    }
    return result;
}

#undef BUFFER_SIZE

bool QHexEdit::isModified()
{
    return _modified;
}

void QHexEdit::redo()
{
    _undoStack->redo();
    refresh();
}

QString QHexEdit::selectionToReadableString()
{
    QByteArray ba;
    for (qint64 i = getSelectionBegin(); i < getSelectionEnd(); i++)
        ba.append(reader ? reader(i) : 0);
    return toReadable(ba);
}

void QHexEdit::setFont(const QFont &font)
{
    QWidget::setFont(font);
    _pxCharWidth = fontMetrics().width(QLatin1Char('2'));
    _pxCharHeight = fontMetrics().height() + 1;
    _pxGapAdr = _pxCharWidth / 2;
    _pxGapAdrHex = _pxCharWidth;
    _pxGapHexAscii = 2 * _pxCharWidth;
    _pxCursorWidth = _pxCharHeight / 7;
    _pxSelectionSub = _pxCharHeight / 5;
    viewport()->update();
}

QString QHexEdit::toReadableString()
{
    QByteArray ba;
    for (qint64 i = 0; i < _editorSize; i++)
        ba.append(reader ? reader(i) : 0);
    return toReadable(ba);
}

void QHexEdit::undo()
{
    _undoStack->undo();
    refresh();
}

// ********************************************************************** Handle events
void QHexEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        event->ignore();
        return;
    }
    // Cursor movements
    if (event->matches(QKeySequence::MoveToNextChar))
    {
        setCursorPosition(_cursorPosition + 1);
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToPreviousChar))
    {
        setCursorPosition(_cursorPosition - 1);
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToEndOfLine))
    {
        setCursorPosition(_cursorPosition | (2 * BYTES_PER_LINE -1));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToStartOfLine))
    {
        setCursorPosition(_cursorPosition - (_cursorPosition % (2 * BYTES_PER_LINE)));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToPreviousLine))
    {
        setCursorPosition(_cursorPosition - (2 * BYTES_PER_LINE));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToNextLine))
    {
        setCursorPosition(_cursorPosition + (2 * BYTES_PER_LINE));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToNextPage))
    {
        setCursorPosition(_cursorPosition + (((_rowsShown - 1) * 2 * BYTES_PER_LINE)));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToPreviousPage))
    {
        setCursorPosition(_cursorPosition - (((_rowsShown - 1) * 2 * BYTES_PER_LINE)));
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToEndOfDocument))
    {
        setCursorPosition(_editorSize * 2);
        resetSelection(_cursorPosition);
    }
    if (event->matches(QKeySequence::MoveToStartOfDocument))
    {
        setCursorPosition(0);
        resetSelection(_cursorPosition);
    }

    // Select commands
    if (event->matches(QKeySequence::SelectAll))
    {
        resetSelection(0);
        setSelection(2*_editorSize + 1);
    }
    if (event->matches(QKeySequence::SelectNextChar))
    {
        qint64 pos = _cursorPosition + 1;
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectPreviousChar))
    {
        qint64 pos = _cursorPosition - 1;
        setSelection(pos);
        setCursorPosition(pos);
    }
    if (event->matches(QKeySequence::SelectEndOfLine))
    {
        qint64 pos = _cursorPosition - (_cursorPosition % (2 * BYTES_PER_LINE)) + (2 * BYTES_PER_LINE);
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectStartOfLine))
    {
        qint64 pos = _cursorPosition - (_cursorPosition % (2 * BYTES_PER_LINE));
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectPreviousLine))
    {
        qint64 pos = _cursorPosition - (2 * BYTES_PER_LINE);
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectNextLine))
    {
        qint64 pos = _cursorPosition + (2 * BYTES_PER_LINE);
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectNextPage))
    {
        qint64 pos = _cursorPosition + (((viewport()->height() / _pxCharHeight) - 1) * 2 * BYTES_PER_LINE);
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectPreviousPage))
    {
        qint64 pos = _cursorPosition - (((viewport()->height() / _pxCharHeight) - 1) * 2 * BYTES_PER_LINE);
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectEndOfDocument))
    {
        qint64 pos = _editorSize * 2;
        setCursorPosition(pos);
        setSelection(pos);
    }
    if (event->matches(QKeySequence::SelectStartOfDocument))
    {
        qint64 pos = 0;
        setCursorPosition(pos);
        setSelection(pos);
    }

    // Edit Commands
    if (!_readOnly)
    {
        if ((QApplication::keyboardModifiers() == Qt::NoModifier) ||
                (QApplication::keyboardModifiers() == Qt::ShiftModifier) ||
                (QApplication::keyboardModifiers() == Qt::KeypadModifier))
        {
            /* Hex input */
            int key = int(event->text()[0].toLatin1());
            if ((key>='0' && key<='9') || (key>='a' && key <= 'f') || (key>='A' && key <= 'F'))
            {
                if (getSelectionBegin() != getSelectionEnd())
                {
                    qint64 len = getSelectionEnd() - getSelectionBegin();
                    replace(getSelectionBegin(), (int)len, QByteArray((int)len, char(0)));
                    
                    setCursorPosition(2*_bPosCurrent);
                    resetSelection(2*_bPosCurrent);
                }

                // Change content
                if (_editorSize > 0)
                {
                    const char byte = reader ? reader(_bPosCurrent) : 0;
                    QByteArray hexValue = QByteArray(&byte, 1).toHex();
                    if ((_cursorPosition % 2) == 0)
                        hexValue[0] = key;
                    else
                        hexValue[1] = key;

                    replace(_bPosCurrent, QByteArray().fromHex(hexValue)[0]);

                    setCursorPosition(_cursorPosition + 1);
                    resetSelection(_cursorPosition);
                }
            }
        }

        /* Cut */
        if (event->matches(QKeySequence::Cut))
        {
            QByteArray bytes;
            for (qint64 i = getSelectionBegin(); i < getSelectionEnd(); i++)
                bytes.append(reader ? reader(i) : 0);
            
            QByteArray ba = bytes.toHex();
            for (qint64 idx = 32; idx < ba.size(); idx +=33)
                ba.insert(idx, "\n");
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(ba);
            qint64 len = getSelectionEnd() - getSelectionBegin();
            replace(getSelectionBegin(), (int)len, QByteArray((int)len, char(0)));
            
            setCursorPosition(2*getSelectionBegin());
            resetSelection(2*getSelectionBegin());
        }

        /* Paste */
        if (event->matches(QKeySequence::Paste))
        {
            QClipboard *clipboard = QApplication::clipboard();
            QByteArray ba = QByteArray().fromHex(clipboard->text().toLatin1());
            replace(_bPosCurrent, ba.size(), ba);
            
            setCursorPosition(_cursorPosition + 2 * ba.size());
            resetSelection(getSelectionBegin());
        }

        /* Delete char */
        if (event->matches(QKeySequence::Delete))
        {
            if (getSelectionBegin() != getSelectionEnd())
            {
                _bPosCurrent = getSelectionBegin();
                QByteArray ba = QByteArray(getSelectionEnd() - getSelectionBegin(), char(0));
                replace(_bPosCurrent, ba.size(), ba);
            }
            else
            {
                replace(_bPosCurrent, char(0));
            }
            setCursorPosition(2 * _bPosCurrent);
            resetSelection(2 * _bPosCurrent);
        }

        /* Backspace */
        if ((event->key() == Qt::Key_Backspace) && (event->modifiers() == Qt::NoModifier))
        {
            if (getSelectionBegin() != getSelectionEnd())
            {
                _bPosCurrent = getSelectionBegin();
                setCursorPosition(2 * _bPosCurrent);
                QByteArray ba = QByteArray(getSelectionEnd() - getSelectionBegin(), char(0));
                replace(_bPosCurrent, ba.size(), ba);
                
                resetSelection(2 * _bPosCurrent);
            }
            else
            {
                _bPosCurrent -= 1;
                replace(_bPosCurrent, char(0));
                
                _bPosCurrent -= 1;
                setCursorPosition(2 * _bPosCurrent);
                resetSelection(2 * _bPosCurrent);
            }
        }

        /* undo */
        if (event->matches(QKeySequence::Undo))
        {
            undo();
        }

        /* redo */
        if (event->matches(QKeySequence::Redo))
        {
            redo();
        }

    }

    /* Copy */
    if (event->matches(QKeySequence::Copy))
    {
        QByteArray bytes;
        for (qint64 i = getSelectionBegin(); i < getSelectionEnd(); i++)
            bytes.append(reader ? reader(i) : 0);
        
        QByteArray ba = bytes.toHex();
        for (qint64 idx = 32; idx < ba.size(); idx +=33)
            ba.insert(idx, "\n");
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(ba);
    }

    refresh();
}

void QHexEdit::mouseMoveEvent(QMouseEvent * event)
{
    _blink = false;
    viewport()->update();
    qint64 actPos = cursorPosition(event->pos());
    if (actPos >= 0)
    {
        setCursorPosition(actPos);
        setSelection(actPos);
    }
}

void QHexEdit::mousePressEvent(QMouseEvent * event)
{
    _blink = false;
    viewport()->update();
    qint64 cPos = cursorPosition(event->pos());
    if (cPos >= 0)
    {
        resetSelection(cPos);
        setCursorPosition(cPos);
    }
}

void QHexEdit::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    if (event->rect() != _cursorRect)
    {
        // process some useful calculations
        int pxOfsX = horizontalScrollBar()->value();
        int pxPosStartY = _pxCharHeight;

        // draw some patterns if needed
        painter.fillRect(event->rect(), viewport()->palette().color(QPalette::Base));
        if (_addressArea)
            painter.fillRect(QRect(-pxOfsX, event->rect().top(), _pxPosHexX - _pxGapAdrHex/2 - pxOfsX, height()), _addressAreaColor);
        if (_asciiArea)
        {
            int linePos = _pxPosAsciiX - (_pxGapHexAscii / 2);
            painter.setPen(Qt::gray);
            painter.drawLine(linePos - pxOfsX, event->rect().top(), linePos - pxOfsX, height());
        }

        painter.setPen(viewport()->palette().color(QPalette::WindowText));

        // paint address area
        if (_addressArea)
        {
            QString address;
            for (int row=0, pxPosY = _pxCharHeight; row < (_dataShown.size()/BYTES_PER_LINE); row++, pxPosY +=_pxCharHeight)
            {
                address = QString("%1").arg(_bPosFirst + row*BYTES_PER_LINE + _addressOffset, _addrDigits, 16, QChar('0'));
                painter.drawText(_pxPosAdrX - pxOfsX, pxPosY, address);
            }
        }

        // paint hex and ascii area
        QPen colStandard = QPen(viewport()->palette().color(QPalette::WindowText));
        // TODO: make these configurable
        QPen colUsageRead   = QPen(QColor(0, 0, 224));
        QPen colUsageWrite  = QPen(QColor(0, 0, 224));
        QPen colUsageExec   = QPen(QColor(224, 0, 0));
        QPen colUsageRWExec = QPen(QColor(224, 0, 224));

        painter.setBackgroundMode(Qt::TransparentMode);

        for (int row = 0, pxPosY = pxPosStartY; row <= _rowsShown; row++, pxPosY +=_pxCharHeight)
        {
            QByteArray hex;
            int pxPosX = _pxPosHexX  - pxOfsX;
            int pxPosAsciiX2 = _pxPosAsciiX  - pxOfsX;
            qint64 bPosLine = row * BYTES_PER_LINE;
            for (int colIdx = 0; ((bPosLine + colIdx) < _dataShown.size() and (colIdx < BYTES_PER_LINE)); colIdx++)
            {
                QColor c = viewport()->palette().color(QPalette::Base);
                painter.setPen(colStandard);

                qint64 posBa = _bPosFirst + bPosLine + colIdx;
                if ((getSelectionBegin() <= posBa) && (getSelectionEnd() > posBa))
                {
                    c = _brushSelection.color();
                    painter.setPen(_penSelection);
                }
                else
                {
                    /* TODO
                    if (_highlighting)
                        if (_markedShown.at((int)(posBa - _bPosFirst)))
                        {
                            c = _brushHighlighted.color();
                            painter.setPen(_penHighlighted);
                        }
                    */
                    
                    uint8_t this_usage = usage(posBa);
                    if (this_usage & UsageExec && this_usage & UsageRead) {
                        painter.setPen(colUsageRWExec);
                    } else if (this_usage & UsageExec) {
                        painter.setPen(colUsageExec);
                    } else if (this_usage & UsageRead) {
                        painter.setPen(colUsageRead);
                    } else if (this_usage & UsageWrite) {
                        painter.setPen(colUsageWrite);
                    }
                }

                // render hex value
                QRect r;
                if (colIdx == 0)
                    r.setRect(pxPosX, pxPosY - _pxCharHeight + _pxSelectionSub, 2*_pxCharWidth, _pxCharHeight);
                else
                    r.setRect(pxPosX - _pxCharWidth, pxPosY - _pxCharHeight + _pxSelectionSub, 3*_pxCharWidth, _pxCharHeight);
                painter.fillRect(r, c);
                hex = _hexDataShown.mid((bPosLine + colIdx) * 2, 2);
                painter.drawText(pxPosX, pxPosY, hex);
                pxPosX += 3*_pxCharWidth;

                // render ascii value
                if (_asciiArea)
                {
                    char ch = _dataShown.at(bPosLine + colIdx);
                    if ((ch < 0x20) or (ch > 0x7e))
                            ch = '.';
                    r.setRect(pxPosAsciiX2, pxPosY - _pxCharHeight + _pxSelectionSub, _pxCharWidth, _pxCharHeight);
                    painter.fillRect(r, c);
                    painter.drawText(pxPosAsciiX2, pxPosY, QChar(ch));
                    pxPosAsciiX2 += _pxCharWidth;
                }
            }
        }
        painter.setBackgroundMode(Qt::TransparentMode);
        painter.setPen(viewport()->palette().color(QPalette::WindowText));
    }

    // paint cursor
    if (_blink && !_readOnly && hasFocus())
        painter.fillRect(_cursorRect, this->palette().color(QPalette::WindowText));
    else
        painter.drawText(_pxCursorX, _pxCursorY, _hexDataShown.mid(_cursorPosition - _bPosFirst * 2, 1));

    // emit event, if size has changed
    if (_lastEventSize != _editorSize)
    {
        _lastEventSize = _editorSize;
        emit currentSizeChanged(_lastEventSize);
    }
}

void QHexEdit::resizeEvent(QResizeEvent *)
{
    adjust();
}

// ********************************************************************** Handle selections
void QHexEdit::resetSelection()
{
    _bSelectionBegin = _bSelectionInit;
    _bSelectionEnd = _bSelectionInit;
}

void QHexEdit::resetSelection(qint64 pos)
{
    if (pos < 0)
        pos = 0;
    pos = pos / 2;
    _bSelectionInit = pos;
    _bSelectionBegin = pos;
    _bSelectionEnd = pos;
}

void QHexEdit::setSelection(qint64 pos)
{
    if (pos < 0)
        pos = 0;
    pos = pos / 2;
    if (pos >= _bSelectionInit)
    {
        _bSelectionEnd = pos;
        _bSelectionBegin = _bSelectionInit;
    }
    else
    {
        _bSelectionBegin = pos;
        _bSelectionEnd = _bSelectionInit;
    }
}

int QHexEdit::getSelectionBegin()
{
    return _bSelectionBegin;
}

int QHexEdit::getSelectionEnd()
{
    return _bSelectionEnd;
}

// ********************************************************************** Private utility functions
void QHexEdit::init()
{
    _undoStack->clear();
    setAddressOffset(0);
    setEditorSize(0);
    resetSelection(0);
    setCursorPosition(0);
    verticalScrollBar()->setValue(0);
    _modified = false;
}

void QHexEdit::adjust()
{
    // recalc Graphics
    if (_addressArea)
    {
        _addrDigits = addressWidth();
        _pxPosHexX = _pxGapAdr + _addrDigits*_pxCharWidth + _pxGapAdrHex;
    }
    else
        _pxPosHexX = _pxGapAdrHex;
    _pxPosAdrX = _pxGapAdr;
    _pxPosAsciiX = _pxPosHexX + HEXCHARS_IN_LINE * _pxCharWidth + _pxGapHexAscii;

    // set horizontalScrollBar()
    int pxWidth = _pxPosAsciiX;
    if (_asciiArea)
        pxWidth += BYTES_PER_LINE*_pxCharWidth;
    horizontalScrollBar()->setRange(0, pxWidth - viewport()->width());
    horizontalScrollBar()->setPageStep(viewport()->width());

    // set verticalScrollbar()
    _rowsShown = ((viewport()->height()-4)/_pxCharHeight);
    int lineCount = (int)(_editorSize / (qint64)BYTES_PER_LINE) + 1;
    verticalScrollBar()->setRange(0, lineCount - _rowsShown);
    verticalScrollBar()->setPageStep(_rowsShown);

    int value = verticalScrollBar()->value();
    _bPosFirst = (qint64)value * BYTES_PER_LINE;
    _bPosLast = _bPosFirst + (qint64)(_rowsShown * BYTES_PER_LINE) - 1;
    if (_bPosLast >= _editorSize)
        _bPosLast = _editorSize - 1;
    readBuffers();
    setCursorPosition(_cursorPosition);
}

void QHexEdit::dataChangedPrivate(int)
{
    _modified = _undoStack->index() != 0;
    adjust();
    emit dataChanged();
}

void QHexEdit::refresh(bool showCursor)
{
    if (showCursor)
        ensureVisible();
    readBuffers();
    viewport()->update();
}

void QHexEdit::readBuffers()
{
    _dataShown.clear();
    for (qint64 i = _bPosFirst; i < qMin(_editorSize, _bPosLast + BYTES_PER_LINE + 1); i++)
        _dataShown.append(reader ? reader(i) : 0);
    
    _hexDataShown = QByteArray(_dataShown.toHex());
}

QString QHexEdit::toReadable(const QByteArray &ba)
{
    QString result;

    for (int i=0; i < ba.size(); i += 16)
    {
        QString addrStr = QString("%1").arg(_addressOffset + i, addressWidth(), 16, QChar('0'));
        QString hexStr;
        QString ascStr;
        for (int j=0; j<16; j++)
        {
            if ((i + j) < ba.size())
            {
                hexStr.append(" ").append(ba.mid(i+j, 1).toHex());
                char ch = ba[i + j];
                if ((ch < 0x20) or (ch > 0x7e))
                        ch = '.';
                ascStr.append(QChar(ch));
            }
        }
        result += addrStr + " " + QString("%1").arg(hexStr, -48) + "  " + QString("%1").arg(ascStr, -17) + "\n";
    }
    return result;
}

void QHexEdit::updateCursor()
{
    if (_blink)
        _blink = false;
    else
        _blink = true;
    viewport()->update(_cursorRect);
}

