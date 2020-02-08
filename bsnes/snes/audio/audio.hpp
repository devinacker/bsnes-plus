class Stream {
public:
  Stream();
  void audio_init();
  alwaysinline bool has_sample();
  alwaysinline uint32 get_sample();

protected:
  void audio_frequency(double frequency);
  void sample(int16 left, int16 right);

private:
  struct {
    uint32 buffer[32768];
    unsigned rdoffset;
    unsigned wroffset;
    unsigned length;

    double r_step, r_frac;
    int r_sum_l, r_sum_r;
  } stream_;
};

class Audio {
public:
  void init();
  void add_stream(Stream* stream);
  void sample(int16 left, int16 right);
  void flush();

private:
  uint32 dsp_buffer[32768];
  unsigned dsp_rdoffset;
  unsigned dsp_wroffset;
  unsigned dsp_length;

  linear_vector<Stream*> streams;
};

extern Audio audio;
