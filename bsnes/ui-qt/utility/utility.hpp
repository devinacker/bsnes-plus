class Utility {
public:
  //utility.cpp
  void inputEvent(uint16_t scancode);
  void showMessage(const char *message);
  void updateSystemState();
  void acquireMouse();
  void unacquireMouse();

  void updateAvSync();
  void updateAvSync(bool syncVideo, bool syncAudio);
  void updateColorFilter();
  void updatePixelShader();
  void updateHardwareFilter();
  void updateSoftwareFilter();
  void updateEmulationSpeed();
  void updateEmulationSpeed(unsigned speed);
  void updateControllers();

  //system-state.cpp
  enum system_state_t { LoadCartridge, UnloadCartridge, PowerOn, PowerOff, PowerCycle, Reset, ReloadCartridge };
  void modifySystemState(system_state_t state);

  //window.cpp
  void updateFullscreenState();
  void constrainSize(unsigned &x, unsigned &y, unsigned max);
  void resizeMainWindow();
  void toggleSynchronizeVideo();
  void toggleSynchronizeAudio();
  void setNtscMode();
  void setPalMode();
  void toggleSmoothVideoOutput();
  void toggleAspectCorrection();
  void setScale(unsigned);
  void toggleFullscreen();
  void toggleMenubar();
  void toggleStatusbar();
};

extern Utility utility;
