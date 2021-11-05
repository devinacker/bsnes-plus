InputGroup userInterfaceGeneral(InputCategory::UserInterface, "General");

namespace UserInterfaceGeneral {

struct ToggleMenubar : HotkeyInput {
  void pressed() {
    config().video.showMenubar ^= true;
    utility.updateMenubar();
  }

  ToggleMenubar() : HotkeyInput("Toggle Menubar", "input.userInterface.general.toggleMenubar") {
    name = "KB0::Tab";
    userInterfaceGeneral.attach(this);
  }
} toggleMenubar;

struct ToggleStatusbar : HotkeyInput {
  void pressed() {
    config().video.showStatusbar ^= true;
    utility.updateStatusbar();
  }

  ToggleStatusbar() : HotkeyInput("Toggle Statusbar", "input.userInterface.general.toggleStatusbar") {
    name = "KB0::Tab";
    userInterfaceGeneral.attach(this);
  }
} toggleStatusbar;

struct ToggleCheatSystem : HotkeyInput {
  void pressed() {
    //there is a signal attached to cheatEnableBox that will update SNES::cheat.enable(bool);
    if(cheatEditorWindow->cheatEnableBox->isChecked() == false) {
      cheatEditorWindow->cheatEnableBox->setChecked(true);
      utility.showMessage("Cheat system enabled.");
    } else {
      cheatEditorWindow->cheatEnableBox->setChecked(false);
      utility.showMessage("Cheat system disabled.");
    }
  }

  ToggleCheatSystem() : HotkeyInput("Toggle Cheat System", "input.userInterface.general.toggleCheatSystem") {
    userInterfaceGeneral.attach(this);
  }
} toggleCheatSystem;

struct CaptureScreenshot : HotkeyInput {
  void pressed() {
    //tell SNES::Interface to save a screenshot at the next video_refresh() event
    intf.saveScreenshot = true;
  }

  CaptureScreenshot() : HotkeyInput("Capture Screenshot", "input.userintf.general.captureScreenshot") {
    userInterfaceGeneral.attach(this);
  }
} captureScreenshot;

struct CaptureSPC : HotkeyInput {
  void pressed() {
    //tell the S-SMP core to save a SPC after the next note-on
    intf.captureSPC();
  }

  CaptureSPC() : HotkeyInput("Capture SPC Dump", "input.userInterface.general.captureSPC") {
    userInterfaceGeneral.attach(this);
  }
} captureSPC;

//put here instead of in a separate "Audio Settings" group,
//because there is only one audio option at present
struct MuteAudioOutput : HotkeyInput {
  void pressed() {
    mainWindow->settings_muteAudio->toggleChecked();
    config().audio.mute = mainWindow->settings_muteAudio->isChecked();
  }

  MuteAudioOutput() : HotkeyInput("Mute Audio Output", "input.userInterface.general.muteAudioOutput") {
    name = "Shift+KB0::M";
    userInterfaceGeneral.attach(this);
  }
} muteAudioOutput;

}
