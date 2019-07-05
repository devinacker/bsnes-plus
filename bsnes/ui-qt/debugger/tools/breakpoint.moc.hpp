class BreakpointModel : public QAbstractTableModel {
  Q_OBJECT

public:
enum {
  BreakAddrStart,
  BreakAddrEnd,
  BreakCompare,
  BreakData,
  BreakRead,
  BreakWrite,
  BreakExecute,
  BreakSource,
  
  BreakColumnCount
};

  static const QStringList sources;
  static const QStringList compares;

  BreakpointModel(QObject *parent = 0);
  ~BreakpointModel() {}

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const;
  
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
  
  Qt::ItemFlags flags(const QModelIndex &index) const;
  
  bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
  QString displayAddr(unsigned addr, SNES::Debugger::Breakpoint::Source source) const;
};

class BreakpointEditor : public Window {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTableView *table;
  BreakpointModel *model;
  QHBoxLayout *btnLayout;
  QPushButton *btnAdd;
  QPushButton *btnRemove;
  QCheckBox *breakOnWDM;
  QCheckBox *breakOnBRK;

  BreakpointEditor();

  void addBreakpoint(const string& addr, const string& mode, const string& source);
  void addBreakpoint(const string& breakpoint);
  void removeBreakpoint(uint32_t index);
  void setBreakOnBrk(bool b);
  string toStrings() const;

  int32_t indexOfBreakpointExec(uint32_t addr, const string &source) const;

public slots:
  void add();
  void remove();
  void toggle();
  void clear();
};

extern BreakpointEditor *breakpointEditor;
