class Symbols;

class SymbolsView : public Window {
  Q_OBJECT

public:
  SymbolsView(class DisasmProcessor *processor);

  QVBoxLayout *layout;
  QTreeWidget *list;

  QLineEdit *search;

public slots:
  void synchronize();
  void bind(QTreeWidgetItem*, int);

private:
  class DisasmProcessor *processor;
};
