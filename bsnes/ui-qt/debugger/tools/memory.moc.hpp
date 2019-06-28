#include "qhexedit2/qhexedit.moc.hpp"

class MemoryEditor : public Window {
  Q_OBJECT

public:
  QGridLayout *layout;
  QHexEdit *editor;
  QVBoxLayout *controlLayout;
  QComboBox *source;
  QLineEdit *addr;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  
  QHBoxLayout *toolLayout;
  QToolButton *prevCodeButton;
  QToolButton *nextCodeButton;
  QToolButton *prevDataButton;
  QToolButton *nextDataButton;
  QToolButton *prevUnkButton;
  QToolButton *nextUnkButton;
  
  QToolButton *findButton;
  QToolButton *findNextButton;
  QToolButton *findPrevButton;
  
  QWidget *spacer;
  QPushButton *exportButton;
  QPushButton *importButton;

  QLabel *statusBar;

  void autoUpdate();
  void synchronize();

  SNES::Debugger::MemorySource memorySource;
  uint8_t reader(unsigned addr);
  void writer(unsigned addr, uint8_t data);
  uint8_t usage(unsigned addr);

  MemoryEditor();

  void closeEvent(QCloseEvent*);

private:
  QByteArray searchStr;
  int searchPos;
  int breakpointPos;

  void addBreakpoint(const string& mode);

  void gotoPrevious(int);
  void gotoNext(int);
  
  void exportMemory(SNES::Memory&, const string&) const;
  void importMemory(SNES::Memory&, const string&) const;
  
public slots:
  void show();
  void refresh();
  
private slots:
  void sourceChanged(int);
  void updateOffset();
  void showAddress(qint64);
  void showContextMenu(const QPoint& pos);
  
  void addBreakpointR();
  void addBreakpointW();
  void addBreakpointX();
  void addBreakpointRW();
  void addBreakpointRWX();
  
  void prevCode();
  void nextCode();
  void prevData();
  void nextData();
  void prevUnknown();
  void nextUnknown();
  
  void search();
  void searchNext();
  void searchPrev();
  
  void exportMemory();
  void importMemory();
};

//extern MemoryEditor *memoryEditor;
extern QVector <MemoryEditor*> memoryEditors;
