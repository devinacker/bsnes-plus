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
  
private:
  QByteArray searchStr;
  int searchPos;

public slots:
  void show();
  void sourceChanged(int);
  void refresh();
  void updateOffset();
  void showAddress(qint64);
  
  void prevCode();
  void nextCode();
  void prevData();
  void nextData();
  void prevUnknown();
  void nextUnknown();
  void gotoPrevious(int);
  void gotoNext(int);
  
  void search();
  void searchNext();
  void searchPrev();
  
  void exportMemory();
  void importMemory();
  void exportMemory(SNES::Memory&, const string&) const;
  void importMemory(SNES::Memory&, const string&) const;
};

extern MemoryEditor *memoryEditor;
