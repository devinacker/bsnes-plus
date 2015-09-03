#include "commands.moc.hpp"
#include <QUndoCommand>

// Helper class to store single byte commands
class CharCommand : public QUndoCommand
{
public:
    CharCommand(QHexEdit *edit, qint64 charPos, char newChar,
                       QUndoCommand *parent=0);

    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *command);
    int id() const { return 1234; }

private:
	QHexEdit *_edit;
    qint64 _charPos;
    bool _wasChanged;
    char _newChar;
    char _oldChar;
};

CharCommand::CharCommand(QHexEdit * edit, qint64 charPos, char newChar, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    _edit = edit;
    _charPos = charPos;
    _newChar = newChar;
}

bool CharCommand::mergeWith(const QUndoCommand *command)
{
    const CharCommand *nextCommand = static_cast<const CharCommand *>(command);
    bool result = false;

    if (nextCommand->_charPos == _charPos)
	{
		_newChar = nextCommand->_newChar;
		result = true;
	}
    return result;
}

void CharCommand::undo()
{
	if (_edit->writer) _edit->writer(_charPos, _oldChar);
	_edit->setCursorPosition(_charPos * 2);
}

void CharCommand::redo()
{
	_oldChar = _edit->reader ? _edit->reader(_charPos) : 0;
	if (_edit->writer) _edit->writer(_charPos, _newChar);
}

UndoStack::UndoStack(QHexEdit * parent)
    : QUndoStack(parent)
{
    _parent = parent;
}

void UndoStack::insert(qint64 pos, char c)
{
    if ((pos >= 0) && (pos <= _parent->editorSize()))
    {
        QUndoCommand *cc = new CharCommand(_parent, pos, c);
        this->push(cc);
    }
}

void UndoStack::insert(qint64 pos, const QByteArray &ba)
{
    if ((pos >= 0) && (pos <= _parent->editorSize()))
    {
        QString txt = QString(tr("Inserting %1 bytes")).arg(ba.size());
        beginMacro(txt);
        for (int idx=0; idx < ba.size(); idx++)
        {
            QUndoCommand *cc = new CharCommand(_parent, pos + idx, ba.at(idx));
            this->push(cc);
        }
        endMacro();
    }
}

void UndoStack::removeAt(qint64 pos, qint64 len)
{
    if ((pos >= 0) && (pos < _parent->editorSize()))
    {
        if (len==1)
        {
            QUndoCommand *cc = new CharCommand(_parent, pos, char(0));
            this->push(cc);
        }
        else
        {
            QString txt = QString(tr("Delete %1 chars")).arg(len);
            beginMacro(txt);
            for (qint64 cnt=0; cnt<len; cnt++)
            {
                QUndoCommand *cc = new CharCommand(_parent, pos, char(0));
                push(cc);
            }
            endMacro();
        }
    }
}

void UndoStack::overwrite(qint64 pos, char c)
{
    if ((pos >= 0) && (pos < _parent->editorSize()))
    {
        QUndoCommand *cc = new CharCommand(_parent, pos, c);
        this->push(cc);
    }
}

void UndoStack::overwrite(qint64 pos, int len, const QByteArray &ba)
{
    if ((pos >= 0) && (pos < _parent->editorSize()))
    {
        QString txt = QString(tr("Overwrite %1 chars")).arg(len);
        beginMacro(txt);
        removeAt(pos, len);
        insert(pos, ba);
        endMacro();
    }
}
