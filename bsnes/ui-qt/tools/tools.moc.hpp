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
  void showCheatEditor()    { tab->setCurrentWidget(cheatEditorArea);    show(); }
  void showCheatFinder()    { tab->setCurrentWidget(cheatFinderArea);    show(); }
  void showStateManager()   { tab->setCurrentWidget(stateManagerArea);   show(); }
  void showEffectToggle()   { tab->setCurrentWidget(effectToggleArea);   show(); }
  void showManifestViewer() { tab->setCurrentWidget(manifestViewerArea); show(); }
};

extern ToolsWindow *toolsWindow;
