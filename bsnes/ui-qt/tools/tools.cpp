#include "../ui-base.hpp"

#include "tools.moc"
ToolsWindow *toolsWindow;

#include "cheateditor.cpp"
#include "cheatfinder.cpp"
#include "statemanager.cpp"
#include "effecttoggle.cpp"

#include "soundviewer.cpp"

ToolsWindow::ToolsWindow() {
  setObjectName("tools-window");
  setWindowTitle("Tools");
  resize(600, 360);
  setGeometryString(&config().geometry.toolsWindow);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setMargin(UIStyle::WindowMargin);
  layout->setSpacing(UIStyle::WidgetSpacing);
  setLayout(layout);

  cheatEditorWindow = new CheatEditorWindow;
  cheatFinderWindow = new CheatFinderWindow;
  stateManagerWindow = new StateManagerWindow;
  effectToggleWindow = new EffectToggleWindow;
  soundViewerWindow = new SoundViewerWindow;
  
  cheatEditorArea = new QScrollArea;
  cheatEditorArea->setWidget(cheatEditorWindow);
  cheatEditorArea->setFrameStyle(0);
  cheatEditorArea->setWidgetResizable(true);

  cheatFinderArea = new QScrollArea;
  cheatFinderArea->setWidget(cheatFinderWindow);
  cheatFinderArea->setFrameStyle(0);
  cheatFinderArea->setWidgetResizable(true);

  stateManagerArea = new QScrollArea;
  stateManagerArea->setWidget(stateManagerWindow);
  stateManagerArea->setFrameStyle(0);
  stateManagerArea->setWidgetResizable(true);

  effectToggleArea = new QScrollArea;
  effectToggleArea->setWidget(effectToggleWindow);
  effectToggleArea->setFrameStyle(0);
  effectToggleArea->setWidgetResizable(true);
  
  tab = new QTabWidget;
  tab->addTab(cheatEditorArea, "Cheat Editor");
  tab->addTab(cheatFinderArea, "Cheat Finder");
  tab->addTab(stateManagerArea, "State Manager");
  if(SNES::PPU::SupportsLayerEnable || SNES::DSP::SupportsChannelEnable)
    tab->addTab(effectToggleArea, "Effect Toggle");

  layout->addWidget(tab);
}
