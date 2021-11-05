class Interface : public SNES::Interface {
public:
  void video_extras(uint16_t *data, unsigned width, unsigned height);
  void video_refresh(const uint16_t *data, unsigned width, unsigned height);
  void audio_sample(uint16_t left, uint16_t right);
  void input_poll();
  int16_t input_poll(bool port, SNES::Input::Device device, unsigned index, unsigned id);
  void message(const string &text);

  Interface();
  void captureScreenshot(const QImage&);
  void captureSPC();
  bool saveScreenshot;
  bool framesUpdated;
  unsigned framesExecuted;
};

extern Interface intf;
