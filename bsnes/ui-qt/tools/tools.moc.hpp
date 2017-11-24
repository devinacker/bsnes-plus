class ToolsWindow : public Window {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTabWidget *tab;
  QScrollArea *cheatEditorArea;
  QScrollArea *cheatFinderArea;
  QScrollArea *stateManagerArea;
  QScrollArea *effectToggleArea;
  QScrollArea *manifestViewerArea;

  ToolsWindow();

public slots:
};

extern ToolsWindow *toolsWindow;
