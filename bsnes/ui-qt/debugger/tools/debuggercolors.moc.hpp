#include "colorpickerbutton/colorpickerbutton.moc.hpp"

class DebuggerColorsWindow : public Window {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QLabel *disassemblerLabel;
  QGridLayout *disassemblerLayout;
  QLabel *disassemblerOpLabel;
  ColorPickerButton *disassemblerOpPicker;
  QLabel *disassemblerParamImmediateLabel;
  ColorPickerButton *disassemblerParamImmediatePicker;
  QLabel *disassemblerParamAddressLabel;
  ColorPickerButton *disassemblerParamAddressPicker;
  QLabel *disassemblerParamSymbolLabel;
  ColorPickerButton *disassemblerParamSymbolPicker;
  QLabel *consoleLabel;
  QGridLayout *consoleLayout;
  QLabel *consoleBreakpointLabel;
  ColorPickerButton *consoleBreakpointPicker;
  QLabel *consoleCpuStepLabel;
  ColorPickerButton *consoleCpuStepPicker;
  QLabel *consoleSmpStepLabel;
  ColorPickerButton *consoleSmpStepPicker;
  QLabel *consoleCoprocessorStepLabel;
  ColorPickerButton *consoleCoprocessorStepPicker;
  QLabel *hexEditorLabel;
  QGridLayout *hexEditorLayout;
  QLabel *hexEditorReadUsageLabel;
  ColorPickerButton *hexEditorReadUsagePicker;
  QLabel *hexEditorWriteUsageLabel;
  ColorPickerButton *hexEditorWriteUsagePicker;
  QLabel *hexEditorExecUsageLabel;
  ColorPickerButton *hexEditorExecUsagePicker;
  QLabel *hexEditorRWEUsageLabel;
  ColorPickerButton *hexEditorRWEUsagePicker;

  void initializeUi();
  DebuggerColorsWindow();

public slots:
  void setDisassemblerOpColor(const QColor &color);
  void setDisassemblerParamImmediateColor(const QColor &color);
  void setDisassemblerParamAddressColor(const QColor &color);
  void setDisassemblerParamSymbolColor(const QColor &color);
  void setConsoleBreakpointColor(const QColor &color);
  void setConsoleCpuStepColor(const QColor &color);
  void setConsoleSmpStepColor(const QColor &color);
  void setConsoleCoprocessorStepColor(const QColor &color);
  void setHexEditorReadUsageColor(const QColor &color);
  void setHexEditorWriteUsageColor(const QColor &color);
  void setHexEditorExecUsageColor(const QColor &color);
  void setHexEditorRWEUsageColor(const QColor &color);

signals:
  void disassemblerColorChanged();
  void hexEditorColorChanged();
};

extern DebuggerColorsWindow *debuggerColors;